/**
 * @file SWE4_TransactionProcessor.cpp
 * @module TransactionProcessor
 * @author AI Test Generator
 * @date 2026-02-28
 * @brief Comprehensive Google Test unit tests for TransactionProcessor class.
 * 
 * This test suite provides comprehensive coverage of the TransactionProcessor class
 * including transaction validation, execution, processing, and logging. External service
 * dependencies are mocked using Google Mock to ensure unit test isolation.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TransactionProcessor.hpp"
#include "ExternalServices.hpp"

using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::testing::_;

// ============================================================================
// MOCK IMPLEMENTATIONS
// ============================================================================

/**
 * Mock implementation of ComplianceCheckService for testing.
 */
class MockComplianceCheckService : public ComplianceCheckService {
public:
    MOCK_METHOD(ComplianceLevel, checkComplianceLevel, (const std::string&), (override));
    MOCK_METHOD(bool, reportSuspiciousActivity, (const std::string&, const std::string&), (override));
    MOCK_METHOD(std::vector<std::string>, getBlacklist, (), (override));
    MOCK_METHOD(bool, isAccountBlacklisted, (const std::string&), (override));
};

/**
 * Mock implementation of AuditLoggingService for testing.
 */
class MockAuditLoggingService : public AuditLoggingService {
public:
    MOCK_METHOD(bool, logTransaction, (const std::string&, const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, logAccountEvent, (const std::string&, const std::string&, const std::string&), (override));
    MOCK_METHOD(std::vector<std::string>, getAuditTrail, (const std::string&), (override));
    MOCK_METHOD(bool, archiveAuditLogs, (const std::string&), (override));
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

/**
 * Test fixture for TransactionProcessor tests.
 * Provides setup and teardown for each test case.
 */
class TransactionProcessorTest : public ::testing::Test {
protected:
    TransactionProcessor processor;
    NiceMock<MockComplianceCheckService> complianceService;
    NiceMock<MockAuditLoggingService> auditService;

    void SetUp() override {
        processor.setComplianceService(&complianceService);
        processor.setAuditService(&auditService);
        processor.resetDailyLimits();
    }

    void TearDown() override {
        processor.resetDailyLimits();
    }
};

// ============================================================================
// VALIDATE TRANSACTION TESTS
// ============================================================================

/**
 * Test: validateTransaction_ValidDeposit_ReturnsTrue
 * Condition: Amount is within valid range and type is DEPOSIT
 * Expected: Function should return true
 */
TEST_F(TransactionProcessorTest, validateTransaction_ValidDeposit_ReturnsTrue) {
    bool result = processor.validateTransaction(1000.0, TransactionType::DEPOSIT);
    
    ASSERT_TRUE(result);
}

/**
 * Test: validateTransaction_BelowMinimumAmount_ReturnsFalse
 * Condition: Amount is below MIN_TRANSACTION_AMOUNT (0.01)
 * Expected: Function should return false
 */
TEST_F(TransactionProcessorTest, validateTransaction_BelowMinimumAmount_ReturnsFalse) {
    bool result = processor.validateTransaction(0.005, TransactionType::DEPOSIT);
    
    ASSERT_FALSE(result);
}

/**
 * Test: validateTransaction_ExactlyMinimumAmount_ReturnsTrue
 * Condition: Amount equals MIN_TRANSACTION_AMOUNT (0.01)
 * Expected: Function should return true
 */
TEST_F(TransactionProcessorTest, validateTransaction_ExactlyMinimumAmount_ReturnsTrue) {
    bool result = processor.validateTransaction(0.01, TransactionType::DEPOSIT);
    
    ASSERT_TRUE(result);
}

/**
 * Test: validateTransaction_AboveMaximumAmount_ReturnsFalse
 * Condition: Amount exceeds MAX_TRANSACTION_AMOUNT (1000000.0)
 * Expected: Function should return false
 */
TEST_F(TransactionProcessorTest, validateTransaction_AboveMaximumAmount_ReturnsFalse) {
    bool result = processor.validateTransaction(1000000.01, TransactionType::TRANSFER);
    
    ASSERT_FALSE(result);
}

/**
 * Test: validateTransaction_ExactlyMaximumAmount_ReturnsTrue
 * Condition: Amount equals MAX_TRANSACTION_AMOUNT (1000000.0)
 * Expected: Function should return true
 */
TEST_F(TransactionProcessorTest, validateTransaction_ExactlyMaximumAmount_ReturnsTrue) {
    bool result = processor.validateTransaction(1000000.0, TransactionType::DEPOSIT);
    
    ASSERT_TRUE(result);
}

/**
 * Test: validateTransaction_LargeWithdrawalExceedsLimit_ReturnsFalse
 * Condition: Type is WITHDRAWAL and amount exceeds 50000.0
 * Expected: Function should return false
 */
TEST_F(TransactionProcessorTest, validateTransaction_LargeWithdrawalExceedsLimit_ReturnsFalse) {
    bool result = processor.validateTransaction(50001.0, TransactionType::WITHDRAWAL);
    
    ASSERT_FALSE(result);
}

/**
 * Test: validateTransaction_LargeWithdrawalAtLimit_ReturnsTrue
 * Condition: Type is WITHDRAWAL and amount equals 50000.0
 * Expected: Function should return true
 */
TEST_F(TransactionProcessorTest, validateTransaction_LargeWithdrawalAtLimit_ReturnsTrue) {
    bool result = processor.validateTransaction(50000.0, TransactionType::WITHDRAWAL);
    
    ASSERT_TRUE(result);
}

/**
 * Test: validateTransaction_RefundExceedsLimit_ReturnsFalse
 * Condition: Type is REFUND and amount exceeds 10000.0
 * Expected: Function should return false
 */
TEST_F(TransactionProcessorTest, validateTransaction_RefundExceedsLimit_ReturnsFalse) {
    bool result = processor.validateTransaction(10001.0, TransactionType::REFUND);
    
    ASSERT_FALSE(result);
}

/**
 * Test: validateTransaction_RefundAtLimit_ReturnsTrue
 * Condition: Type is REFUND and amount equals 10000.0
 * Expected: Function should return true
 */
TEST_F(TransactionProcessorTest, validateTransaction_RefundAtLimit_ReturnsTrue) {
    bool result = processor.validateTransaction(10000.0, TransactionType::REFUND);
    
    ASSERT_TRUE(result);
}

// ============================================================================
// EXECUTE TRANSFER TESTS
// ============================================================================

/**
 * Test: executeTransfer_EmptySourceAccount_ReturnsRejected
 * Condition: Source account is empty string
 * Expected: Function should return REJECTED
 */
TEST_F(TransactionProcessorTest, executeTransfer_EmptySourceAccount_ReturnsRejected) {
    TransactionStatus result = processor.executeTransfer(1000.0, "", "ACC123", false);
    
    ASSERT_EQ(result, TransactionStatus::REJECTED);
}

/**
 * Test: executeTransfer_EmptyDestinationAccount_ReturnsRejected
 * Condition: Destination account is empty string
 * Expected: Function should return REJECTED
 */
TEST_F(TransactionProcessorTest, executeTransfer_EmptyDestinationAccount_ReturnsRejected) {
    TransactionStatus result = processor.executeTransfer(1000.0, "ACC123", "", false);
    
    ASSERT_EQ(result, TransactionStatus::REJECTED);
}

/**
 * Test: executeTransfer_SameSourceDestination_ZeroAmount_ReturnsCancelled
 * Condition: Source equals destination and amount is 0.0
 * Expected: Function should return CANCELLED
 */
TEST_F(TransactionProcessorTest, executeTransfer_SameSourceDestination_ZeroAmount_ReturnsCancelled) {
    TransactionStatus result = processor.executeTransfer(0.0, "ACC123", "ACC123", false);
    
    ASSERT_EQ(result, TransactionStatus::CANCELLED);
}

/**
 * Test: executeTransfer_SameSourceDestination_PositiveAmount_ReturnsRejected
 * Condition: Source equals destination and amount > 0.0
 * Expected: Function should return REJECTED
 */
TEST_F(TransactionProcessorTest, executeTransfer_SameSourceDestination_PositiveAmount_ReturnsRejected) {
    TransactionStatus result = processor.executeTransfer(1000.0, "ACC123", "ACC123", false);
    
    ASSERT_EQ(result, TransactionStatus::REJECTED);
}

/**
 * Test: executeTransfer_ValidTransfer_ReturnsCompleted
 * Condition: Valid transfer with proper amounts and accounts
 * Expected: Function should return COMPLETED or APPROVED
 */
TEST_F(TransactionProcessorTest, executeTransfer_ValidTransfer_ReturnsCompleted) {
    TransactionStatus result = processor.executeTransfer(1000.0, "ACC123", "ACC456", false);
    
    ASSERT_TRUE(result == TransactionStatus::COMPLETED || result == TransactionStatus::APPROVED);
}

/**
 * Test: executeTransfer_NegativeAmount_ReturnsCancelled
 * Condition: Amount is negative
 * Expected: Function should return CANCELLED
 */
TEST_F(TransactionProcessorTest, executeTransfer_NegativeAmount_ReturnsCancelled) {
    TransactionStatus result = processor.executeTransfer(-1000.0, "ACC123", "ACC456", false);
    
    ASSERT_EQ(result, TransactionStatus::CANCELLED);
}

// ============================================================================
// PROCESS TRANSACTION TESTS
// ============================================================================

/**
 * Test: processTransaction_InvalidAmount_ReturnsRejected
 * Condition: Amount below MIN_TRANSACTION_AMOUNT
 * Expected: Function should return REJECTED
 */
TEST_F(TransactionProcessorTest, processTransaction_InvalidAmount_ReturnsRejected) {
    TransactionStatus result = processor.processTransaction(
        TransactionType::DEPOSIT, 0.001, "ACC123", "");
    
    ASSERT_EQ(result, TransactionStatus::REJECTED);
}

/**
 * Test: processTransaction_BlockedCompliance_ReturnsRejected
 * Condition: Compliance service returns BLOCKED level
 * Expected: Function should return REJECTED
 */
TEST_F(TransactionProcessorTest, processTransaction_BlockedCompliance_ReturnsRejected) {
    EXPECT_CALL(complianceService, checkComplianceLevel("ACC123"))
        .WillOnce(Return(ComplianceLevel::BLOCKED));
    
    TransactionStatus result = processor.processTransaction(
        TransactionType::TRANSFER, 1000.0, "ACC123", "ACC456");
    
    ASSERT_EQ(result, TransactionStatus::REJECTED);
}

/**
 * Test: processTransaction_HighRiskHighAmount_ReturnsRejected
 * Condition: Compliance level is HIGH_RISK and amount > 50000.0
 * Expected: Function should return REJECTED
 */
TEST_F(TransactionProcessorTest, processTransaction_HighRiskHighAmount_ReturnsRejected) {
    EXPECT_CALL(complianceService, checkComplianceLevel("ACC123"))
        .WillOnce(Return(ComplianceLevel::HIGH_RISK));
    
    TransactionStatus result = processor.processTransaction(
        TransactionType::TRANSFER, 60000.0, "ACC123", "ACC456");
    
    ASSERT_EQ(result, TransactionStatus::REJECTED);
}

/**
 * Test: processTransaction_DepositValidTransaction_ReturnsCompleted
 * Condition: Valid deposit transaction
 * Expected: Function should return COMPLETED
 */
TEST_F(TransactionProcessorTest, processTransaction_DepositValidTransaction_ReturnsCompleted) {
    TransactionStatus result = processor.processTransaction(
        TransactionType::DEPOSIT, 1000.0, "ACC123", "");
    
    ASSERT_EQ(result, TransactionStatus::COMPLETED);
}

/**
 * Test: processTransaction_WithdrawalValidTransaction_ReturnsCompleted
 * Condition: Valid withdrawal transaction
 * Expected: Function should return COMPLETED
 */
TEST_F(TransactionProcessorTest, processTransaction_WithdrawalValidTransaction_ReturnsCompleted) {
    TransactionStatus result = processor.processTransaction(
        TransactionType::WITHDRAWAL, 1000.0, "ACC123", "");
    
    ASSERT_EQ(result, TransactionStatus::COMPLETED);
}

/**
 * Test: processTransaction_RefundSmallAmount_ReturnsCompleted
 * Condition: Valid refund with amount <= 10000.0
 * Expected: Function should return COMPLETED
 */
TEST_F(TransactionProcessorTest, processTransaction_RefundSmallAmount_ReturnsCompleted) {
    TransactionStatus result = processor.processTransaction(
        TransactionType::REFUND, 5000.0, "ACC123", "");
    
    ASSERT_EQ(result, TransactionStatus::COMPLETED);
}

/**
 * Test: processTransaction_RefundLargeAmount_ReturnsRejected
 * Condition: Valid refund with amount > 10000.0
 * Expected: Function should return REJECTED (validation fails for large refunds)
 */
TEST_F(TransactionProcessorTest, processTransaction_RefundLargeAmount_ReturnsRejected) {
    TransactionStatus result = processor.processTransaction(
        TransactionType::REFUND, 15000.0, "ACC123", "");
    
    ASSERT_EQ(result, TransactionStatus::REJECTED);
}

// ============================================================================
// RESET DAILY LIMITS TESTS
// ============================================================================

/**
 * Test: resetDailyLimits_InitialCall_ReturnsTrue
 * Condition: Reset is called
 * Expected: Function should return true
 */
TEST_F(TransactionProcessorTest, resetDailyLimits_InitialCall_ReturnsTrue) {
    bool result = processor.resetDailyLimits();
    
    ASSERT_TRUE(result);
}

/**
 * Test: resetDailyLimits_ResetsVolume_ToZero
 * Condition: Reset is called after transactions
 * Expected: Daily volume should be reset to 0.0
 */
TEST_F(TransactionProcessorTest, resetDailyLimits_ResetsVolume_ToZero) {
    processor.processTransaction(TransactionType::DEPOSIT, 1000.0, "ACC123", "");
    ASSERT_GT(processor.getDailyVolume(), 0.0);
    
    processor.resetDailyLimits();
    
    ASSERT_EQ(processor.getDailyVolume(), 0.0);
}

/**
 * Test: resetDailyLimits_ResetsTransactionCount_ToZero
 * Condition: Reset is called after transactions
 * Expected: Daily transaction count should be reset to 0
 */
TEST_F(TransactionProcessorTest, resetDailyLimits_ResetsTransactionCount_ToZero) {
    processor.processTransaction(TransactionType::DEPOSIT, 1000.0, "ACC123", "");
    ASSERT_GT(processor.getTransactionCount(), 0);
    
    processor.resetDailyLimits();
    
    ASSERT_EQ(processor.getTransactionCount(), 0);
}

// ============================================================================
// GET DAILY VOLUME TESTS
// ============================================================================

/**
 * Test: getDailyVolume_InitialValue_ReturnsZero
 * Condition: No transactions have been processed
 * Expected: Daily volume should be 0.0
 */
TEST_F(TransactionProcessorTest, getDailyVolume_InitialValue_ReturnsZero) {
    double volume = processor.getDailyVolume();
    
    ASSERT_EQ(volume, 0.0);
}

/**
 * Test: getDailyVolume_AfterDeposit_ReturnsUpdatedVolume
 * Condition: Deposit transaction is processed
 * Expected: Daily volume should be updated
 */
TEST_F(TransactionProcessorTest, getDailyVolume_AfterDeposit_ReturnsUpdatedVolume) {
    processor.processTransaction(TransactionType::DEPOSIT, 1500.0, "ACC123", "");
    
    double volume = processor.getDailyVolume();
    
    ASSERT_GT(volume, 0.0);
}

/**
 * Test: getDailyVolume_AfterMultipleTransactions_Accumulates
 * Condition: Multiple transactions are processed
 * Expected: Daily volume should accumulate
 */
TEST_F(TransactionProcessorTest, getDailyVolume_AfterMultipleTransactions_Accumulates) {
    processor.processTransaction(TransactionType::DEPOSIT, 1000.0, "ACC123", "");
    processor.processTransaction(TransactionType::DEPOSIT, 2000.0, "ACC456", "");
    
    double volume = processor.getDailyVolume();
    
    ASSERT_GE(volume, 3000.0);
}

// ============================================================================
// GET TRANSACTION COUNT TESTS
// ============================================================================

/**
 * Test: getTransactionCount_InitialValue_ReturnsZero
 * Condition: No transactions have been processed
 * Expected: Transaction count should be 0
 */
TEST_F(TransactionProcessorTest, getTransactionCount_InitialValue_ReturnsZero) {
    int count = processor.getTransactionCount();
    
    ASSERT_EQ(count, 0);
}

/**
 * Test: getTransactionCount_AfterTransaction_ReturnsUpdatedCount
 * Condition: Transaction is processed successfully
 * Expected: Transaction count should increment
 */
TEST_F(TransactionProcessorTest, getTransactionCount_AfterTransaction_ReturnsUpdatedCount) {
    processor.processTransaction(TransactionType::DEPOSIT, 1000.0, "ACC123", "");
    
    int count = processor.getTransactionCount();
    
    ASSERT_EQ(count, 1);
}

/**
 * Test: getTransactionCount_AfterMultipleTransactions_ReturnsCorrectCount
 * Condition: Multiple transactions are processed
 * Expected: Transaction count should reflect all transactions
 */
TEST_F(TransactionProcessorTest, getTransactionCount_AfterMultipleTransactions_ReturnsCorrectCount) {
    processor.processTransaction(TransactionType::DEPOSIT, 1000.0, "ACC123", "");
    processor.processTransaction(TransactionType::DEPOSIT, 2000.0, "ACC456", "");
    processor.processTransaction(TransactionType::WITHDRAWAL, 500.0, "ACC789", "");
    
    int count = processor.getTransactionCount();
    
    ASSERT_EQ(count, 3);
}

/**
 * Test: getTransactionCount_RejectedTransactions_NotCounted
 * Condition: Invalid/rejected transactions are attempted
 * Expected: Rejected transactions should not increment counter
 */
TEST_F(TransactionProcessorTest, getTransactionCount_RejectedTransactions_NotCounted) {
    processor.processTransaction(TransactionType::DEPOSIT, 0.001, "ACC123", "");
    processor.processTransaction(TransactionType::DEPOSIT, 1000.0, "ACC123", "");
    
    int count = processor.getTransactionCount();
    
    // Only the valid transaction should be counted
    ASSERT_EQ(count, 1);
}
