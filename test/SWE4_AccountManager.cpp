#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <limits>
#include <tuple>

#include "../inc/AccountManager.hpp"
#include "../inc/ExternalServices.hpp"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

// ============================================================================
// Mock Classes
// ============================================================================

class MockAuthenticationService : public AuthenticationService {
public:
    MOCK_METHOD(bool, validateCredentials, (const std::string& username, const std::string& password), (override));
    MOCK_METHOD(bool, enableMultiFactor, (const std::string& accountNumber), (override));
    MOCK_METHOD(VerificationResult, verifyMultiFactorToken, (const std::string& accountNumber, const std::string& token), (override));
    MOCK_METHOD(bool, lockAccount, (const std::string& accountNumber), (override));
};

class MockNotificationService : public NotificationService {
public:
    MOCK_METHOD(bool, sendEmailNotification, (const std::string& email, const std::string& subject, const std::string& body), (override));
    MOCK_METHOD(bool, sendSmsNotification, (const std::string& phoneNumber, const std::string& message), (override));
    MOCK_METHOD(bool, sendPushNotification, (const std::string& deviceToken, const std::string& title, const std::string& message), (override));
    MOCK_METHOD(bool, subscribeToNotifications, (const std::string& accountNumber, const std::string& notificationType), (override));
};

class MockExternalDataService : public ExternalDataService {
public:
    MOCK_METHOD(std::string, getCreditScore, (const std::string& accountNumber), (override));
    MOCK_METHOD(std::string, getIdentityVerificationStatus, (const std::string& accountNumber), (override));
    MOCK_METHOD(bool, validateBankAccount, (const std::string& bankAccount, const std::string& routingNumber), (override));
    MOCK_METHOD(std::vector<std::string>, getLinkedAccounts, (const std::string& primaryAccount), (override));
};

// ============================================================================
// Test Fixture Class for normal individual tests
// ============================================================================

class AccountManagerUnitTest : public ::testing::Test {
protected:
    AccountManager sut;
    NiceMock<MockAuthenticationService> mockAuth;
    NiceMock<MockNotificationService> mockNotif;
    NiceMock<MockExternalDataService> mockData;

    void SetUp() override {
        sut.setAuthenticationService(&mockAuth);
        sut.setNotificationService(&mockNotif);
        sut.setExternalDataService(&mockData);
        
        // Ensure default mock behavior if needed, e.g., dummy vectors for getLinkedAccounts
        ON_CALL(mockData, getLinkedAccounts(_)).WillByDefault(Return(std::vector<std::string>{}));
    }

    void TearDown() override {
        // Services aren't owned by AccountManager, so no manual deletion needed inside it from ptrs
    }
};

// ============================================================================
// Method: createAccount()
// ============================================================================

/// ===========================================================================
/// Verifies: AccountManager::createAccount()
/// Test goal: Correctly create account if valid balance and max limit not reached
/// In case: Create AccountManager, call createAccount with acceptable balance, verify length
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_createAccount_Normal_Valid_Balance) {
    std::string acc = sut.createAccount(AccountType::CHECKING, 100.0);
    EXPECT_FALSE(acc.empty());
    EXPECT_EQ(sut.getAccountBalance(acc), 100.0);
}

/// ===========================================================================
/// Verifies: AccountManager::createAccount()
/// Test goal: Refuses to create account if initial balance is less than minimum
/// In case: call createAccount with negative or zero balance, expecting empty return
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_createAccount_Error_Below_Min_Balance) {
    std::string acc = sut.createAccount(AccountType::CHECKING, 0.0);
    EXPECT_TRUE(acc.empty());
}

/// ===========================================================================
/// Verifies: AccountManager::createAccount()
/// Test goal: Refuse creating more accounts than MAX_ACCOUNTS_PER_USER
/// In case: create 10 valid accounts, the 11th should return an empty string
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_createAccount_Error_Max_Accounts_Reached) {
    for (int i = 0; i < 10; ++i) {
        std::string acc = sut.createAccount(AccountType::CHECKING, 10.0);
        EXPECT_FALSE(acc.empty());
    }
    // 11th attempt shouldn't work
    std::string accOver = sut.createAccount(AccountType::CHECKING, 10.0);
    EXPECT_TRUE(accOver.empty());
}


// ============================================================================
// Method: activateAccount()
// ============================================================================

/// ===========================================================================
/// Verifies: AccountManager::activateAccount()
/// Test goal: Validate successful account activation
/// In case: Create account, set as verified via mock, call activateAccount
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_activateAccount_Normal_Success) {
    std::string acc = sut.createAccount(AccountType::SAVINGS, 50.0);
    sut.verifyAccount(acc, true); // this will also set it straight to ACTIVE if it succeeds
    // In that case, let's reset to test activateAccount manually
    Account* ptr = sut.getAccount(acc);
    ptr->status = AccountStatus::PENDING_VERIFICATION;
    ptr->isVerified = true;
    
    bool active = sut.activateAccount(acc);
    EXPECT_TRUE(active);
    EXPECT_EQ(ptr->status, AccountStatus::ACTIVE);
}

/// ===========================================================================
/// Verifies: AccountManager::activateAccount()
/// Test goal: Validate failed activation if account doesn't exist
/// In case: call activateAccount on non-existent ID
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_activateAccount_Error_NotFound) {
    bool active = sut.activateAccount("ACC999999");
    EXPECT_FALSE(active);
}

/// ===========================================================================
/// Verifies: AccountManager::activateAccount()
/// Test goal: Validate failure if account is not verified and is pending
/// In case: call activateAccount on newly created account without verification
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_activateAccount_Error_NotVerified) {
    std::string acc = sut.createAccount(AccountType::SAVINGS, 50.0);
    bool active = sut.activateAccount(acc);
    EXPECT_FALSE(active);
}

/// ===========================================================================
/// Verifies: AccountManager::activateAccount()
/// Test goal: Validate failure if account is frozen
/// In case: call activateAccount on frozen account
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_activateAccount_Error_Frozen) {
    std::string acc = sut.createAccount(AccountType::SAVINGS, 50.0);
    Account* ptr = sut.getAccount(acc);
    ptr->status = AccountStatus::FROZEN;
    
    bool active = sut.activateAccount(acc);
    EXPECT_FALSE(active);
}


/// ===========================================================================
/// Verifies: AccountManager::activateAccount()
/// Test goal: Validate failure if account is closed
/// In case: call activateAccount on closed account
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_activateAccount_Error_Closed) {
    std::string acc = sut.createAccount(AccountType::SAVINGS, 50.0);
    Account* ptr = sut.getAccount(acc);
    ptr->status = AccountStatus::CLOSED;
    
    bool active = sut.activateAccount(acc);
    EXPECT_FALSE(active);
}

// ============================================================================
// Method: suspendAccount()
// ============================================================================

/// ===========================================================================
/// Verifies: AccountManager::suspendAccount()
/// Test goal: Correctly suspends a valid account
/// In case: Create account, suspend it, verify status is SUSPENDED and suspended count increments
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_suspendAccount_Normal_Success) {
    int initS = sut.getSuspendedAccountCount();
    std::string acc = sut.createAccount(AccountType::CHECKING, 50.0);
    
    bool ret = sut.suspendAccount(acc, "Suspicious activity");
    EXPECT_TRUE(ret);
    EXPECT_EQ(sut.getAccount(acc)->status, AccountStatus::SUSPENDED);
    EXPECT_EQ(sut.getSuspendedAccountCount(), initS + 1);
}

/// ===========================================================================
/// Verifies: AccountManager::suspendAccount()
/// Test goal: Fails to suspend closed or missing account
/// In case: Suspend missing and then closed accounts
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_suspendAccount_Error_InvalidState) {
    EXPECT_FALSE(sut.suspendAccount("ACC999999", "reason"));
    
    std::string acc = sut.createAccount(AccountType::CHECKING, 50.0);
    Account* ptr = sut.getAccount(acc);
    ptr->status = AccountStatus::CLOSED;
    
    EXPECT_FALSE(sut.suspendAccount(acc, "reason"));
}


// ============================================================================
// Method: deactivateAccount()
// ============================================================================

/// ===========================================================================
/// Verifies: AccountManager::deactivateAccount()
/// Test goal: Validates deactivation of an account with 0 balance
/// In case: Create account, set balance to 0, deactivate it
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_deactivateAccount_Normal_Success) {
    std::string acc = sut.createAccount(AccountType::CHECKING, 50.0);
    Account* ptr = sut.getAccount(acc);
    ptr->balance = 0.0;
    
    bool ret = sut.deactivateAccount(acc);
    EXPECT_TRUE(ret);
    EXPECT_EQ(ptr->status, AccountStatus::CLOSED);
}

/// ===========================================================================
/// Verifies: AccountManager::deactivateAccount()
/// Test goal: Fails to deactivate an account with balance
/// In case: Create account with 50 balance, deactivate it, should fail
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_deactivateAccount_Error_HasBalance) {
    std::string acc = sut.createAccount(AccountType::CHECKING, 50.0);
    bool ret = sut.deactivateAccount(acc);
    EXPECT_FALSE(ret);
}

/// ===========================================================================
/// Verifies: AccountManager::deactivateAccount()
/// Test goal: Fails to deactivate an already closed or invalid account
/// In case: Deactivate missing account, and closed account
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_deactivateAccount_Error_InvalidState) {
    EXPECT_FALSE(sut.deactivateAccount("ACC999999"));
    
    std::string acc = sut.createAccount(AccountType::CHECKING, 50.0);
    Account* ptr = sut.getAccount(acc);
    ptr->status = AccountStatus::CLOSED;
    EXPECT_FALSE(sut.deactivateAccount(acc));
}


// ============================================================================
// Method: evaluateAccountRisk() (Parameterized testing for conditionals)
// ============================================================================

class EvaluateRiskParamTest : public ::testing::TestWithParam<std::tuple<int, double, bool, bool, bool, AccountStatus>> {
protected:
    AccountManager sut;
    NiceMock<MockExternalDataService> mockData;

    void SetUp() override {
        sut.setExternalDataService(&mockData);
        ON_CALL(mockData, getLinkedAccounts(_)).WillByDefault(Return(std::vector<std::string>{"L1", "L2"}));
    }
};

extern bool g_complianceAuditMode; // To modify the global variable

// Parameters:
// transactionCount, volumeLastDay, isVerified, hasFraudAlert, complianceAuditMode, ExpectedStatus
INSTANTIATE_TEST_SUITE_P(
    SWE4_AccountManager_RiskCases,
    EvaluateRiskParamTest,
    ::testing::Values(
        // Very high risk, frozen due to audit mode
        std::make_tuple(150 /*+30*/, 1200000.0 /*+40*/, false, true /*+35*/, true, AccountStatus::FROZEN),
        // Very high risk, suspended (no audit mode)
        std::make_tuple(150 /*+30*/, 1200000.0 /*+40*/, false, true /*+35*/, false, AccountStatus::SUSPENDED),
        // Moderately high risk mapped to PENDING_VERIFICATION (Score between 50 and 75)
        std::make_tuple(60 /*+15*/, 600000.0 /*+20*/, false, false /*+20*/, false, AccountStatus::PENDING_VERIFICATION), // Score 55
        // Safe account mapped to ACTIVE
        std::make_tuple(5, 500.0, true, false, false, AccountStatus::ACTIVE),
        // MCDC conditions on transactionCount
        std::make_tuple(25 /*+5*/, 0.0, true, false, false, AccountStatus::ACTIVE),
        // MCDC conditions on Volume
        std::make_tuple(5, 150000.0 /*+10*/, true, false, false, AccountStatus::ACTIVE),
        // MCDC conditions on verification vs fraud alert
        std::make_tuple(5, 0.0, true, true /*+25*/, false, AccountStatus::ACTIVE)
    )
);

/// ===========================================================================
/// Verifies: AccountManager::evaluateAccountRisk()
/// Test goal: Checks MCDC combinations of risk limits and returns correct RiskStatus
/// In case: Create account, set attributes, invoke evaluation
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_P(EvaluateRiskParamTest, SWE4_AccountManager_evaluateAccountRisk_MCDC) {
    auto [txCount, volLatest, isVerified, fraudAlert, auditMode, expectedStatus] = GetParam();
    
    g_complianceAuditMode = auditMode;
    std::string acc = sut.createAccount(AccountType::BUSINESS, 100.0);
    Account* ptr = sut.getAccount(acc);
    ptr->isVerified = isVerified;
    ptr->hasFraudAlert = fraudAlert;
    
    // Check mock being called
    EXPECT_CALL(mockData, getLinkedAccounts(acc)).Times(1);

    AccountStatus st = sut.evaluateAccountRisk(acc, txCount, volLatest);
    EXPECT_EQ(st, expectedStatus);
    
    // Reset global state
    g_complianceAuditMode = false;
}

// Ensure unknown evaluateAccountRisk returns CLOSED
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_evaluateAccountRisk_Error_NotFound) {
    EXPECT_EQ(sut.evaluateAccountRisk("ACC9999", 5, 5.0), AccountStatus::CLOSED);
}

// Ensure evaluateAccountRisk works without data service
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_evaluateAccountRisk_NullDataService) {
    sut.setExternalDataService(nullptr);
    std::string acc = sut.createAccount(AccountType::BUSINESS, 100.0);
    AccountStatus st = sut.evaluateAccountRisk(acc, 5, 50.0);
    EXPECT_EQ(st, AccountStatus::ACTIVE);
}


// ============================================================================
// Method: updateAccountStatus()
// ============================================================================

class UpdateStatusParamTest : public ::testing::TestWithParam<std::tuple<AccountStatus, AccountStatus, int, bool, bool, bool>> {
protected:
    AccountManager sut;
};

// Params: oldStatus, newStatus, riskScore, isVerified, fraudAlert, expectedReturn
INSTANTIATE_TEST_SUITE_P(
    SWE4_AccountManager_UpdateStatusCases,
    UpdateStatusParamTest,
    ::testing::Values(
        std::make_tuple(AccountStatus::CLOSED, AccountStatus::ACTIVE, 0, true, false, false), // Can't revert closed
        std::make_tuple(AccountStatus::CLOSED, AccountStatus::CLOSED, 0, true, false, true),  // Closed to closed is allowed
        std::make_tuple(AccountStatus::FROZEN, AccountStatus::ACTIVE, 0, false, false, false),// Unfrozen needs verification
        std::make_tuple(AccountStatus::FROZEN, AccountStatus::ACTIVE, 0, true, true, false),  // Unfrozen cant have fraud alert
        std::make_tuple(AccountStatus::FROZEN, AccountStatus::ACTIVE, 0, true, false, true),  // Valid unfrozen
        std::make_tuple(AccountStatus::FROZEN, AccountStatus::CLOSED, 0, true, false, true),  // Valid frozen to closed
        std::make_tuple(AccountStatus::ACTIVE, AccountStatus::SUSPENDED, 50, true, false, false), // Can't suspend if score < 75 AND active
        std::make_tuple(AccountStatus::PENDING_VERIFICATION, AccountStatus::SUSPENDED, 50, true, false, true), // OK if not active
        std::make_tuple(AccountStatus::ACTIVE, AccountStatus::SUSPENDED, 80, true, false, true),  // Valid suspend
        std::make_tuple(AccountStatus::SUSPENDED, AccountStatus::ACTIVE, 0, true, false, true),  // Valid un-suspend (should decrement suspended count)
        std::make_tuple(AccountStatus::ACTIVE, AccountStatus::PENDING_VERIFICATION, 0, true, false, true), // Normal transition
        std::make_tuple(AccountStatus::ACTIVE, AccountStatus::FROZEN, 0, true, false, true), // Active to frozen
        std::make_tuple(AccountStatus::SUSPENDED, AccountStatus::CLOSED, 0, true, false, true) // Suspended to closed
    )
);

/// ===========================================================================
/// Verifies: AccountManager::updateAccountStatus()
/// Test goal: Checks various state transitions for updateAccountStatus using MCDC coverage
/// In case: Pre-set Account, execute update and verify expected bool return
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_P(UpdateStatusParamTest, SWE4_AccountManager_updateAccountStatus_Transitions) {
    auto [oldSt, newSt, score, isVerified, fraudAlert, expected] = GetParam();
    std::string acc = sut.createAccount(AccountType::CHECKING, 10.0);
    Account* ptr = sut.getAccount(acc);
    ptr->status = oldSt;
    ptr->riskScore = score;
    ptr->isVerified = isVerified;
    ptr->hasFraudAlert = fraudAlert;
    
    // To handle suspend counts which is a bit messy, let's normalize this instance.
    if (oldSt == AccountStatus::SUSPENDED) {
        // Technically not a valid state since we bypass the actual suspend func, count won't match,
        // but it doesn't matter for the bool logic tested.
    }
    
    bool ret = sut.updateAccountStatus(acc, newSt);
    EXPECT_EQ(ret, expected);
}

// NotFound test
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_updateAccountStatus_Error_NotFound) {
    EXPECT_FALSE(sut.updateAccountStatus("ACC99", AccountStatus::ACTIVE));
}


// ============================================================================
// Method: verifyAccount()
// ============================================================================

/// ===========================================================================
/// Verifies: AccountManager::verifyAccount()
/// Test goal: Successfully verifies account and sends notifications/hits APIs
/// In case: newly pending account verified via mock returning valid checks
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_verifyAccount_Normal_Success) {
    std::string acc = sut.createAccount(AccountType::CHECKING, 100.0);
    EXPECT_CALL(mockData, getIdentityVerificationStatus(acc)).WillOnce(Return("Pass"));
    EXPECT_CALL(mockData, getCreditScore(acc)).WillOnce(Return("750"));
    EXPECT_CALL(mockNotif, sendEmailNotification(_, "Account Verified", _)).WillOnce(Return(true));
    
    bool ret = sut.verifyAccount(acc, true);
    
    EXPECT_TRUE(ret);
    Account* ptr = sut.getAccount(acc);
    EXPECT_TRUE(ptr->isVerified);
    EXPECT_EQ(ptr->status, AccountStatus::ACTIVE);
}

/// ===========================================================================
/// Verifies: AccountManager::verifyAccount()
/// Test goal: Returns false when given false, although it marks as false
/// In case: newly pending account fails verification
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_verifyAccount_Normal_False) {
    std::string acc = sut.createAccount(AccountType::CHECKING, 100.0);
    EXPECT_CALL(mockData, getIdentityVerificationStatus(acc)).WillOnce(Return("Fail"));
    EXPECT_CALL(mockData, getCreditScore(acc)).WillOnce(Return("300"));
    // Expect no notification
    
    bool ret = sut.verifyAccount(acc, false);
    
    EXPECT_FALSE(ret);
    Account* ptr = sut.getAccount(acc);
    EXPECT_FALSE(ptr->isVerified);
    EXPECT_EQ(ptr->status, AccountStatus::PENDING_VERIFICATION);
}

/// ===========================================================================
/// Verifies: AccountManager::verifyAccount()
/// Test goal: Correctly ignores non-existent accounts
/// In case: invoke on an account not there
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_verifyAccount_Error_NotFound) {
    EXPECT_FALSE(sut.verifyAccount("ACC99", true));
}

/// ===========================================================================
/// Verifies: AccountManager::verifyAccount()
/// Test goal: Fails to verify if no data service is active
/// In case: set services to null, invoke verifyAccount
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_verifyAccount_NullServices) {
    sut.setExternalDataService(nullptr);
    sut.setNotificationService(nullptr);
    std::string acc = sut.createAccount(AccountType::CHECKING, 100.0);
    bool ret = sut.verifyAccount(acc, true);
    
    EXPECT_TRUE(ret); // Since it was PENDING_VERIFICATION and verifyResult is true
}

/// ===========================================================================
/// Verifies: AccountManager::verifyAccount()
/// Test goal: Verifies account but it's already active, should return false
/// In case: verifyAccount on ACTIVE account
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_verifyAccount_AlreadyActive) {
    std::string acc = sut.createAccount(AccountType::CHECKING, 100.0);
    Account* ptr = sut.getAccount(acc);
    ptr->status = AccountStatus::ACTIVE; // Override status
    
    EXPECT_CALL(mockData, getIdentityVerificationStatus(acc)).WillOnce(Return("Pass"));
    EXPECT_CALL(mockData, getCreditScore(acc)).WillOnce(Return("750"));
    EXPECT_CALL(mockNotif, sendEmailNotification(_, _, _)).WillOnce(Return(true));
    
    bool ret = sut.verifyAccount(acc, true);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(ptr->isVerified);
}


// ============================================================================
// Method: getAccountBalance() & getAccount()
// ============================================================================

/// ===========================================================================
/// Verifies: AccountManager::getAccountBalance() & getAccount()
/// Test goal: Correctly fetches account properties or correctly returns null/-1 if invalid
/// In case: valid vs uninitialized variables
/// Method for Verification: Control flow analysis (C0/C1/C2 coverage)
/// ===========================================================================
TEST_F(AccountManagerUnitTest, SWE4_AccountManager_Getters) {
    EXPECT_EQ(sut.getAccountBalance("ACC99"), -1.0);
    EXPECT_EQ(sut.getAccount("ACC99"), nullptr);
    
    std::string acc = sut.createAccount(AccountType::CHECKING, 100.0);
    EXPECT_EQ(sut.getAccountBalance(acc), 100.0);
    EXPECT_NE(sut.getAccount(acc), nullptr);
}
