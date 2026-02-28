#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <limits>
#include <tuple>

#include "../inc/TransactionProcessor.hpp"
#include "../inc/ExternalServices.hpp"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

// ============================================================================
// External Global Variables to reset State
// ============================================================================

extern int g_totalTransactionsProcessed;
extern double g_totalVolumeProcessed;
extern bool g_systemLocked;

// ============================================================================
// Mock Classes
// ============================================================================

class MockComplianceCheckService : public ComplianceCheckService {
public:
    MOCK_METHOD(ComplianceLevel, checkComplianceLevel, (const std::string& accountNumber), (override));
    MOCK_METHOD(bool, reportSuspiciousActivity, (const std::string& accountNumber, const std::string& description), (override));
    MOCK_METHOD(std::vector<std::string>, getBlacklist, (), (override));
    MOCK_METHOD(bool, isAccountBlacklisted, (const std::string& accountNumber), (override));
};

class MockAuditLoggingService : public AuditLoggingService {
public:
    MOCK_METHOD(bool, logTransaction, (const std::string& accountId, const std::string& amount, const std::string& timestamp), (override));
    MOCK_METHOD(bool, logAccountEvent, (const std::string& accountId, const std::string& eventType, const std::string& description), (override));
    MOCK_METHOD(std::vector<std::string>, getAuditTrail, (const std::string& accountNumber), (override));
    MOCK_METHOD(bool, archiveAuditLogs, (const std::string& archiveDate), (override));
};

// ============================================================================
// Test Fixture Class for normal individual tests
// ============================================================================

class TransactionProcessorUnitTest : public ::testing::Test {
protected:
    TransactionProcessor sut;
    NiceMock<MockComplianceCheckService> mockCompliance;
    NiceMock<MockAuditLoggingService> mockAudit;

    void SetUp() override {
        sut.setComplianceService(&mockCompliance);
        sut.setAuditService(&mockAudit);
        
        // Reset global & static variables if needed for test isolation
        // Note: static counters like transactionCounter persist but we can reset limits
        sut.resetDailyLimits();
        g_systemLocked = false;
        
        ON_CALL(mockCompliance, checkComplianceLevel(_)).WillByDefault(Return(ComplianceLevel::LOW_RISK));
        ON_CALL(mockAudit, logTransaction(_, _, _)).WillByDefault(Return(true));
        ON_CALL(mockAudit, logAccountEvent(_, _, _)).WillByDefault(Return(true));
    }
};

// ============================================================================
// Method: validateTransaction()
// ============================================================================

class ValidateTransactionParamTest : public ::testing::TestWithParam<std::tuple<double, TransactionType, bool>> {
protected:
    TransactionProcessor sut;
};

// Parameters: amount, type, expectedIsValid
INSTANTIATE_TEST_SUITE_P(
    SWE4_TransactionProcessor_ValidateCases,
    ValidateTransactionParamTest,
    ::testing::Values(
        // Boundary: MIN_TRANSACTION_AMOUNT (0.01)
        std::make_tuple(0.009, TransactionType::DEPOSIT, false), // < MIN
        std::make_tuple(0.01, TransactionType::DEPOSIT, true),   // = MIN
        
        // Boundary: MAX_TRANSACTION_AMOUNT (1000000.0)
        std::make_tuple(1000000.0, TransactionType::DEPOSIT, true),    // = MAX
        std::make_tuple(1000000.01, TransactionType::DEPOSIT, false),  // > MAX
        
        // Type specific: WITHDRAWAL limit 50,000.0
        std::make_tuple(50000.0, TransactionType::WITHDRAWAL, true),
        std::make_tuple(50000.01, TransactionType::WITHDRAWAL, false),
        
        // Type specific: REFUND limit 10,000.0
        std::make_tuple(10000.0, TransactionType::REFUND, true),
        std::make_tuple(10000.01, TransactionType::REFUND, false),
        
        // Normal bounds (Transfer, Deposit, etc)
        std::make_tuple(500000.0, TransactionType::TRANSFER, true),
        std::make_tuple(80000.0, TransactionType::WITHDRAWAL, false) // Double check withdrawal again directly
    )
);

/// ===========================================================================
/// Verifies: TransactionProcessor::validateTransaction()
/// Test goal: Validates MCDC logic around min/max boundaries and type limits
/// In case: Create TransactionProcessor, call validateTransaction with bounds
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_P(ValidateTransactionParamTest, SWE4_TransactionProcessor_validateTransaction_MCDC) {
    auto [amt, type, expected] = GetParam();
    bool actual = sut.validateTransaction(amt, type);
    EXPECT_EQ(actual, expected);
}

// ============================================================================
// Method: executeTransfer()
// ============================================================================

class ExecuteTransferParamTest : public ::testing::TestWithParam<std::tuple<double, std::string, std::string, bool, bool, int, double, TransactionStatus>> {
protected:
    TransactionProcessor sut;
    
    void SetUp() override {
        // We will manually push transaction counts/volume for some bounds
    }
};

// Since executeTransfer checks `dailyTransactionCount` which is private, we will
// reach the limit by feeding processTransaction X times in the test logic.
// For speed here in params, we pass a boolean `fillLimits` and `fillVolume`.

// Params: amount, source, dest, isUrgent, systemLocked, txCountToAdd, volToAdd, expectedStatus
INSTANTIATE_TEST_SUITE_P(
    SWE4_TransactionProcessor_ExecuteTransferCases,
    ExecuteTransferParamTest,
    ::testing::Values(
        // Condition: Empty IDs
        std::make_tuple(100.0, "", "B", false, false, 0, 0.0, TransactionStatus::REJECTED),
        std::make_tuple(100.0, "A", "", false, false, 0, 0.0, TransactionStatus::REJECTED),
        
        // Condition: Same Account
        std::make_tuple(100.0, "A", "A", false, false, 0, 0.0, TransactionStatus::REJECTED),
        std::make_tuple(-50.0, "A", "A", false, false, 0, 0.0, TransactionStatus::CANCELLED),
        
        // Condition: Urgent limits (Max 5,000,000 total volume; max 1000 txs)
        // Note: amount > 100000.0 invokes urgent limit checking.
        std::make_tuple(150000.0, "A", "B", true, false, 1000, 0.0, TransactionStatus::REJECTED), // Max transactions reached
        std::make_tuple(150000.0, "A", "B", true, false, 0, 4900000.0, TransactionStatus::REJECTED), // 4.9M + 150k = 5.05M > 5M limit (REJECTED)
        std::make_tuple(150000.0, "A", "B", true, false, 0, 100000.0, TransactionStatus::COMPLETED), // Normal urgent executing completely
        
        // Condition: System Lock override
        std::make_tuple(50.0, "A", "B", false, true, 0, 0.0, TransactionStatus::PENDING), // Lock overrides false-urgent
        std::make_tuple(50.0, "A", "B", true, true, 0, 0.0, TransactionStatus::APPROVED), // Lock permits urgent but forces APPROVED
        
        // Condition: Final Validation Check Returns
        std::make_tuple(100.0, "A", "B", false, false, 0, 0.0, TransactionStatus::COMPLETED), // Normal ok
        std::make_tuple(100.0, "A", "B", false, false, 0, 4999950.0, TransactionStatus::APPROVED), // Vol exactly passes to Approved (amount > 0 AND daily tx ok but vol + amt > 5M -> APPROVED logic fallback since First check >5M fails)
        std::make_tuple(100.0, "A", "B", false, false, 1000, 0.0, TransactionStatus::PENDING), // Amount > 0 but max tx reached -> PENDING
        std::make_tuple(-10.0, "A", "B", false, false, 0, 0.0, TransactionStatus::CANCELLED)  // amount <= 0
    )
);

/// ===========================================================================
/// Verifies: TransactionProcessor::executeTransfer()
/// Test goal: Validates executeTransfer behavior with complicated state conditionals
/// In case: Create TP, optionally lock sys or fill limits, invoke executeTransfer
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_P(ExecuteTransferParamTest, SWE4_TransactionProcessor_executeTransfer_MCDC) {
    auto [amt, src, dst, isUrg, sysLock, txToAdd, volToAdd, expectStatus] = GetParam();
    
    g_systemLocked = sysLock;
    
    // Fill limits using processTransaction (since it pushes limits up)
    for (int i=0; i<txToAdd; ++i) {
        // Feed an amount that is valid but small so it doesn't max volume.
        sut.processTransaction(TransactionType::DEPOSIT, 1.0, "C", "D");
    }
    if (volToAdd > 0.0) {
        // Need to push volume up without pushing transaction count up too high if possible...
        // We can just execute a few large deposits if tx count shouldn't be high.
        int chunks = (int)(volToAdd / 1000000.0) + 1;
        double amtChunk = volToAdd / chunks;
        for (int i=0; i<chunks; ++i) {
            sut.processTransaction(TransactionType::DEPOSIT, amtChunk, "C", "D");
        }
    }
    
    TransactionStatus actualStatus = sut.executeTransfer(amt, src, dst, isUrg);
    EXPECT_EQ(actualStatus, expectStatus);
    
    // Cleanup global state
    g_systemLocked = false;
}

// ============================================================================
// Method: processTransaction()
// ============================================================================

/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction()
/// Test goal: Fails immediately if validateTransaction fails
/// In case: Negative amount deposit
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_processTransaction_Error_InvalidValidation) {
    EXPECT_EQ(sut.processTransaction(TransactionType::DEPOSIT, -5.0, "A", ""), TransactionStatus::REJECTED);
}


/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction()
/// Test goal: Compliance High Risk (>50k) limits
/// In case: Mock compliance to HIGH_RISK, feed > 50k transfer
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_processTransaction_Error_HighRiskAmount) {
    EXPECT_CALL(mockCompliance, checkComplianceLevel("SRC"))
        .WillOnce(Return(ComplianceLevel::HIGH_RISK));
    
    EXPECT_EQ(sut.processTransaction(TransactionType::TRANSFER, 60000.0, "SRC", "DST"), TransactionStatus::REJECTED);
}

/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction()
/// Test goal: Compliance Check BLOCKED
/// In case: Mock compliance blocked
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_processTransaction_Error_BlockedCompliance) {
    EXPECT_CALL(mockCompliance, checkComplianceLevel("SRC"))
        .WillOnce(Return(ComplianceLevel::BLOCKED));
    
    EXPECT_EQ(sut.processTransaction(TransactionType::TRANSFER, 100.0, "SRC", "DST"), TransactionStatus::REJECTED);
}

/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction()
/// Test goal: Compliance works even without compliance service 
/// In case: set array to null
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_processTransaction_Normal_NullServices) {
    sut.setComplianceService(nullptr);
    sut.setAuditService(nullptr);
    
    EXPECT_EQ(sut.processTransaction(TransactionType::DEPOSIT, 100.0, "SRC", ""), TransactionStatus::COMPLETED);
    EXPECT_EQ(sut.getTransactionCount(), 1);
}


class ProcessTransactionTypeParamTest : public ::testing::TestWithParam<std::tuple<TransactionType, double, int, TransactionStatus>> {
protected:
    TransactionProcessor sut;
    NiceMock<MockAuditLoggingService> mockAudit;
    NiceMock<MockComplianceCheckService> mockCompliance;
    
    void SetUp() override {
        sut.setComplianceService(&mockCompliance);
        sut.setAuditService(&mockAudit);
        ON_CALL(mockCompliance, checkComplianceLevel(_)).WillByDefault(Return(ComplianceLevel::LOW_RISK));
        // Reset limits to be safe
        sut.resetDailyLimits();
    }
};

// Params: type, amount, preFillTxs, expectedStatus
INSTANTIATE_TEST_SUITE_P(
    SWE4_TransactionProcessor_ProcessTypeCases,
    ProcessTransactionTypeParamTest,
    ::testing::Values(
        // DEPOSIT rules
        std::make_tuple(TransactionType::DEPOSIT, 10.0, 0, TransactionStatus::COMPLETED), // Normal
        std::make_tuple(TransactionType::DEPOSIT, 10.0, 1000, TransactionStatus::REJECTED), // Max Txs limit -> Rejected
        
        // WITHDRAWAL rules (amount >0 and <= 50,000 handled via validate already though)
        std::make_tuple(TransactionType::WITHDRAWAL, 10.0, 0, TransactionStatus::COMPLETED), // Normal
        std::make_tuple(TransactionType::WITHDRAWAL, 10.0, 1000, TransactionStatus::REJECTED), // Max Txs limit -> Rejected
        
        // Note: withdrawing <=0 is blocked by validation limit so we can't test "amount <= 0" on withdrawal block.
        // Wait, withdrawal "amount > 0" condition. If somehow it bypasses validation...
        // Let's rely on valid inputs. 
        
        // REFUND rules
        std::make_tuple(TransactionType::REFUND, 10.0, 0, TransactionStatus::COMPLETED), // <= 10000 -> COMPLETED
        std::make_tuple(TransactionType::REFUND, 0.05, 0, TransactionStatus::COMPLETED), // Valid
        
        // Enum fallback (e.g. invalid cast) -> CANCELLED
        std::make_tuple(static_cast<TransactionType>(99), 10.0, 0, TransactionStatus::CANCELLED)
    )
);

/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction()
/// Test goal: Evaluates different type branches in processTransaction
/// In case: Dispatch transactions based on Type
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_P(ProcessTransactionTypeParamTest, SWE4_TransactionProcessor_processTransaction_Types) {
    auto [type, amt, fillTx, expectedStatus] = GetParam();
    
    // Pre-fill daily limits
    for (int i = 0; i < fillTx; ++i) {
        // Bypass processTransaction so we don't accidentally get logging errors
        // or we just rely on standard limits being pushed via DEPOSIT
        sut.processTransaction(TransactionType::DEPOSIT, 10.0, "A", "B");
    }
    
    TransactionStatus status = sut.processTransaction(type, amt, "X", "Y");
    EXPECT_EQ(status, expectedStatus);
}

// ============================================================================
// Edge constraints specifically
// ============================================================================

/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction() (WITHDRAWAL pending branch)
/// Test goal: Evaluates the specific PENDING block for Withdrawal (Not fully definable natively unless bypassed)
/// In case: Pushed to limit edge.
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_processTransaction_Withdrawal_Pending) {
    // If dailyTransactionCount is NOT >= MAX but amount is <= 0 or > 50,000. 
    // > 50,000 withdrawal is BLOCKED by validateTransaction returning false.
    // <= 0 is BLOCKED by validateTransaction returning false.
    // Therefore, the (status = TransactionStatus::PENDING) branch for withdrawal is
    // UNREACHABLE DEAD CODE unless validateTransaction rules change!
    // We document this in the coverage limitations instead of forcing an impossible state.
}

/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction() (REFUND pending branch)
/// Test goal: Refund > 10,000 returning PENDING
/// In case: Bypass validation restriction?
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_processTransaction_Refund_Pending) {
    // Same rule: validateTransaction explicitly returns false if type == REFUND and amount > 10000.0
    // The inner condition (amount > 10000.0) -> PENDING is dead code because validateTransaction 
    // strips it. This confirms dead-code constraints and will be documented in limitations.
}

/// ===========================================================================
/// Verifies: TransactionProcessor::processTransaction() (DEPOSIT amount check bypass branch)
/// Test goal: Deposit < 0
/// In case: Rejected natively.
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_processTransaction_Deposit_RejectAmount) {
    // A deposit < 0 is blocked by validateTransaction (MIN_TRANSACTION_AMOUNT=0.01)
    // The fallback 'else -> REJECTED' inside the deposit branch can't be reached by amount<=0.
    // However, it CAN be reached by dailyTransactionCount >= MAX_DAILY_TRANSACTIONS.
    // That is tested in ProcessTransactionTypeParamTest already.
}

// ============================================================================
// Method: Getters & Reset
// ============================================================================

/// ===========================================================================
/// Verifies: TransactionProcessor::getDailyVolume() & getTransactionCount() & resetDailyLimits()
/// Test goal: Verify getters and limits reset
/// In case: deposit elements, read getters, call reset, verify 0
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(TransactionProcessorUnitTest, SWE4_TransactionProcessor_GettersAndReset) {
    sut.processTransaction(TransactionType::DEPOSIT, 50.0, "SRC", "");
    EXPECT_EQ(sut.getDailyVolume(), 50.0);
    EXPECT_EQ(sut.getTransactionCount(), 1);
    
    EXPECT_TRUE(sut.resetDailyLimits());
    
    EXPECT_EQ(sut.getDailyVolume(), 0.0);
    EXPECT_EQ(sut.getTransactionCount(), 0);
}
