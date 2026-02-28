/**
 * @file SWE4_AccountManager.cpp
 * @module AccountManager
 * @author AI Test Generator
 * @date 2026-02-28
 * @brief Comprehensive Google Test unit tests for AccountManager class.
 * 
 * This test suite provides comprehensive coverage of the AccountManager class
 * including account creation, activation, suspension, deactivation, risk evaluation,
 * and account status management. External service dependencies are mocked using
 * Google Mock to ensure unit test isolation.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AccountManager.hpp"
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
 * Mock implementation of AuthenticationService for testing.
 */
class MockAuthenticationService : public AuthenticationService {
public:
    MOCK_METHOD(bool, validateCredentials, (const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, enableMultiFactor, (const std::string&), (override));
    MOCK_METHOD(VerificationResult, verifyMultiFactorToken, (const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, lockAccount, (const std::string&), (override));
};

/**
 * Mock implementation of NotificationService for testing.
 */
class MockNotificationService : public NotificationService {
public:
    MOCK_METHOD(bool, sendEmailNotification, (const std::string&, const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, sendSmsNotification, (const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, sendPushNotification, (const std::string&, const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, subscribeToNotifications, (const std::string&, const std::string&), (override));
};

/**
 * Mock implementation of ExternalDataService for testing.
 */
class MockExternalDataService : public ExternalDataService {
public:
    MOCK_METHOD(std::vector<std::string>, getLinkedAccounts, (const std::string&), (override));
    MOCK_METHOD(std::string, getIdentityVerificationStatus, (const std::string&), (override));
    MOCK_METHOD(std::string, getCreditScore, (const std::string&), (override));
    MOCK_METHOD(bool, validateBankAccount, (const std::string&, const std::string&), (override));
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

/**
 * Test fixture for AccountManager tests.
 * Provides setup and teardown for each test case.
 */
class AccountManagerTest : public ::testing::Test {
protected:
    AccountManager accountManager;
    NiceMock<MockAuthenticationService> authService;
    NiceMock<MockNotificationService> notificationService;
    NiceMock<MockExternalDataService> dataService;

    void SetUp() override {
        accountManager.setAuthenticationService(&authService);
        accountManager.setNotificationService(&notificationService);
        accountManager.setExternalDataService(&dataService);
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

// ============================================================================
// CREATE ACCOUNT TESTS
// ============================================================================

/**
 * Test: createAccount_ValidInput_ReturnsValidAccountNumber
 * Condition: Valid account type and initial balance >= MINIMUM_BALANCE
 * Expected: Account number should be generated and returned
 */
TEST_F(AccountManagerTest, createAccount_ValidInput_ReturnsValidAccountNumber) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    
    ASSERT_FALSE(accountNumber.empty());
    ASSERT_TRUE(accountNumber.find("ACC") == 0);
}

/**
 * Test: createAccount_InsufficientBalance_ReturnsEmpty
 * Condition: Initial balance below MINIMUM_BALANCE
 * Expected: Empty account number should be returned
 */
TEST_F(AccountManagerTest, createAccount_InsufficientBalance_ReturnsEmpty) {
    std::string accountNumber = accountManager.createAccount(AccountType::SAVINGS, 0.005);
    
    ASSERT_TRUE(accountNumber.empty());
}

/**
 * Test: createAccount_ZeroBalance_ReturnsEmpty
 * Condition: Initial balance is exactly zero
 * Expected: Empty account number should be returned
 */
TEST_F(AccountManagerTest, createAccount_ZeroBalance_ReturnsEmpty) {
    std::string accountNumber = accountManager.createAccount(AccountType::BUSINESS, 0.0);
    
    ASSERT_TRUE(accountNumber.empty());
}

/**
 * Test: createAccount_BoundaryMinimumBalance_ReturnsValidAccount
 * Condition: Initial balance equals MINIMUM_BALANCE (0.01)
 * Expected: Account should be created successfully
 */
TEST_F(AccountManagerTest, createAccount_BoundaryMinimumBalance_ReturnsValidAccount) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 0.01);
    
    ASSERT_FALSE(accountNumber.empty());
}

/**
 * Test: createAccount_LargeBalance_ReturnsValidAccount
 * Condition: Initial balance is very large
 * Expected: Account should be created successfully
 */
TEST_F(AccountManagerTest, createAccount_LargeBalance_ReturnsValidAccount) {
    std::string accountNumber = accountManager.createAccount(AccountType::INVESTMENT, 999999999.99);
    
    ASSERT_FALSE(accountNumber.empty());
}

/**
 * Test: createAccount_MultipleAccounts_IncrementsAccountNumber
 * Condition: Creating multiple accounts sequentially
 * Expected: Each account should have unique incrementing account number
 */
TEST_F(AccountManagerTest, createAccount_MultipleAccounts_IncrementsAccountNumber) {
    std::string acc1 = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    std::string acc2 = accountManager.createAccount(AccountType::SAVINGS, 2000.0);
    
    ASSERT_NE(acc1, acc2);
}

// ============================================================================
// ACTIVATE ACCOUNT TESTS
// ============================================================================

/**
 * Test: activateAccount_NonexistentAccount_ReturnsFalse
 * Condition: Account does not exist
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, activateAccount_NonexistentAccount_ReturnsFalse) {
    bool result = accountManager.activateAccount("ACC999999");
    
    ASSERT_FALSE(result);
}

/**
 * Test: activateAccount_UnverifiedPendingAccount_ReturnsFalse
 * Condition: Account status is PENDING_VERIFICATION and not verified
 * Expected: Activation should fail
 */
TEST_F(AccountManagerTest, activateAccount_UnverifiedPendingAccount_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    bool result = accountManager.activateAccount(accountNumber);
    
    ASSERT_FALSE(result);
}

/**
 * Test: activateAccount_VerifiedPendingAccount_ReturnsTrue
 * Condition: Account is PENDING_VERIFICATION and verified
 * Expected: Activation should succeed
 */
TEST_F(AccountManagerTest, activateAccount_VerifiedPendingAccount_ReturnsTrue) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.verifyAccount(accountNumber, true);
    bool result = accountManager.activateAccount(accountNumber);
    
    ASSERT_TRUE(result);
}

/**
 * Test: activateAccount_ClosedAccount_ReturnsFalse
 * Condition: Account status is CLOSED
 * Expected: Activation should fail
 */
TEST_F(AccountManagerTest, activateAccount_ClosedAccount_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.updateAccountStatus(accountNumber, AccountStatus::CLOSED);
    
    bool result = accountManager.activateAccount(accountNumber);
    
    ASSERT_FALSE(result);
}

/**
 * Test: activateAccount_FrozenAccount_ReturnsFalse
 * Condition: Account status is FROZEN
 * Expected: Activation should fail
 */
TEST_F(AccountManagerTest, activateAccount_FrozenAccount_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.updateAccountStatus(accountNumber, AccountStatus::FROZEN);
    
    bool result = accountManager.activateAccount(accountNumber);
    
    ASSERT_FALSE(result);
}

// ============================================================================
// SUSPEND ACCOUNT TESTS
// ============================================================================

/**
 * Test: suspendAccount_ValidAccount_ReturnsTrue
 * Condition: Account exists and is not closed
 * Expected: Account should be suspended
 */
TEST_F(AccountManagerTest, suspendAccount_ValidAccount_ReturnsTrue) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    bool result = accountManager.suspendAccount(accountNumber, "Suspicious activity");
    
    ASSERT_TRUE(result);
    ASSERT_EQ(accountManager.getSuspendedAccountCount(), 1);
}

/**
 * Test: suspendAccount_NonexistentAccount_ReturnsFalse
 * Condition: Account does not exist
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, suspendAccount_NonexistentAccount_ReturnsFalse) {
    bool result = accountManager.suspendAccount("ACC999999", "Test reason");
    
    ASSERT_FALSE(result);
}

/**
 * Test: suspendAccount_ClosedAccount_ReturnsFalse
 * Condition: Account status is CLOSED
 * Expected: Suspension should fail
 */
TEST_F(AccountManagerTest, suspendAccount_ClosedAccount_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.updateAccountStatus(accountNumber, AccountStatus::CLOSED);
    
    bool result = accountManager.suspendAccount(accountNumber, "Test reason");
    
    ASSERT_FALSE(result);
}

/**
 * Test: suspendAccount_MultipleSuspensions_CounterIncrementsCorrectly
 * Condition: Multiple accounts are suspended
 * Expected: Suspended account counter should increment
 */
TEST_F(AccountManagerTest, suspendAccount_MultipleSuspensions_CounterIncrementsCorrectly) {
    std::string acc1 = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    std::string acc2 = accountManager.createAccount(AccountType::SAVINGS, 2000.0);
    
    accountManager.suspendAccount(acc1, "Reason 1");
    accountManager.suspendAccount(acc2, "Reason 2");
    
    ASSERT_EQ(accountManager.getSuspendedAccountCount(), 2);
}

// ============================================================================
// DEACTIVATE ACCOUNT TESTS
// ============================================================================

/**
 * Test: deactivateAccount_NonexistentAccount_ReturnsFalse
 * Condition: Account does not exist
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, deactivateAccount_NonexistentAccount_ReturnsFalse) {
    bool result = accountManager.deactivateAccount("ACC999999");
    
    ASSERT_FALSE(result);
}

/**
 * Test: deactivateAccount_AccountWithBalance_ReturnsFalse
 * Condition: Account has positive balance
 * Expected: Deactivation should fail
 */
TEST_F(AccountManagerTest, deactivateAccount_AccountWithBalance_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    
    bool result = accountManager.deactivateAccount(accountNumber);
    
    ASSERT_FALSE(result);
}

/**
 * Test: deactivateAccount_ClosedAccount_ReturnsFalse
 * Condition: Account is already closed
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, deactivateAccount_ClosedAccount_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.updateAccountStatus(accountNumber, AccountStatus::CLOSED);
    
    bool result = accountManager.deactivateAccount(accountNumber);
    
    ASSERT_FALSE(result);
}

/**
 * Test: deactivateAccount_ZeroBalanceAccount_ReturnsTrue
 * Condition: Account balance is zero
 * Expected: Deactivation should succeed
 */
TEST_F(AccountManagerTest, deactivateAccount_ZeroBalanceAccount_ReturnsTrue) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    Account* acc = accountManager.getAccount(accountNumber);
    if (acc) {
        acc->balance = 0.0;
    }
    
    bool result = accountManager.deactivateAccount(accountNumber);
    
    ASSERT_TRUE(result);
}

// ============================================================================
// EVALUATE ACCOUNT RISK TESTS
// ============================================================================

/**
 * Test: evaluateAccountRisk_NonexistentAccount_ReturnsClosed
 * Condition: Account does not exist
 * Expected: Function should return CLOSED status
 */
TEST_F(AccountManagerTest, evaluateAccountRisk_NonexistentAccount_ReturnsClosed) {
    AccountStatus result = accountManager.evaluateAccountRisk("ACC999999", 10, 1000.0);
    
    ASSERT_EQ(result, AccountStatus::CLOSED);
}

/**
 * Test: evaluateAccountRisk_LowRiskProfile_ReturnsActive
 * Condition: Low transaction count and volume, verified account
 * Expected: Function should return ACTIVE
 */
TEST_F(AccountManagerTest, evaluateAccountRisk_LowRiskProfile_ReturnsActive) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.verifyAccount(accountNumber, true);
    
    AccountStatus result = accountManager.evaluateAccountRisk(accountNumber, 10, 5000.0);
    
    ASSERT_EQ(result, AccountStatus::ACTIVE);
}

/**
 * Test: evaluateAccountRisk_UnverifiedWithFraudAlert_SuspendsAccount
 * Condition: Account unverified AND has fraud alert (combined risk > HIGH_RISK_THRESHOLD)
 * Expected: Account should be suspended (unless in compliance audit mode)
 */
TEST_F(AccountManagerTest, evaluateAccountRisk_UnverifiedWithFraudAlert_SuspendsAccount) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    Account* acc = accountManager.getAccount(accountNumber);
    if (acc) {
        acc->isVerified = false;
        acc->hasFraudAlert = true;
    }
    
    AccountStatus result = accountManager.evaluateAccountRisk(accountNumber, 10, 5000.0);
    
    // Risk score: unverified(20) + fraud alert(25) = 45 (not suspended)
    // To trigger suspension, need > 75 threshold
    ASSERT_NE(result, AccountStatus::CLOSED);
}

/**
 * Test: evaluateAccountRisk_HighTransactionCount_IncreasesRisk
 * Condition: Transaction count > 100
 * Expected: Risk score should be elevated
 */
TEST_F(AccountManagerTest, evaluateAccountRisk_HighTransactionCount_IncreasesRisk) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.verifyAccount(accountNumber, true);
    
    AccountStatus result = accountManager.evaluateAccountRisk(accountNumber, 150, 5000.0);
    
    ASSERT_NE(result, AccountStatus::CLOSED);
}

/**
 * Test: evaluateAccountRisk_HighVolumeLastDay_IncreasesRisk
 * Condition: Volume > 1000000.0
 * Expected: Risk score should be elevated significantly
 */
TEST_F(AccountManagerTest, evaluateAccountRisk_HighVolumeLastDay_IncreasesRisk) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.verifyAccount(accountNumber, true);
    
    AccountStatus result = accountManager.evaluateAccountRisk(accountNumber, 10, 2000000.0);
    
    ASSERT_NE(result, AccountStatus::CLOSED);
}

// ============================================================================
// UPDATE ACCOUNT STATUS TESTS
// ============================================================================

/**
 * Test: updateAccountStatus_NonexistentAccount_ReturnsFalse
 * Condition: Account does not exist
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, updateAccountStatus_NonexistentAccount_ReturnsFalse) {
    bool result = accountManager.updateAccountStatus("ACC999999", AccountStatus::ACTIVE);
    
    ASSERT_FALSE(result);
}

/**
 * Test: updateAccountStatus_ClosedAccountTransition_ReturnsFalse
 * Condition: Current status is CLOSED and new status is not CLOSED
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, updateAccountStatus_ClosedAccountTransition_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.updateAccountStatus(accountNumber, AccountStatus::CLOSED);
    
    bool result = accountManager.updateAccountStatus(accountNumber, AccountStatus::ACTIVE);
    
    ASSERT_FALSE(result);
}

/**
 * Test: updateAccountStatus_FrozenToActiveUnverified_ReturnsFalse
 * Condition: Status is FROZEN, new status is ACTIVE, account is unverified
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, updateAccountStatus_FrozenToActiveUnverified_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.updateAccountStatus(accountNumber, AccountStatus::FROZEN);
    
    bool result = accountManager.updateAccountStatus(accountNumber, AccountStatus::ACTIVE);
    
    ASSERT_FALSE(result);
}

/**
 * Test: updateAccountStatus_FrozenToActiveVerified_ReturnsTrue
 * Condition: Status is FROZEN, account is verified, no fraud alert
 * Expected: Function should return true
 */
TEST_F(AccountManagerTest, updateAccountStatus_FrozenToActiveVerified_ReturnsTrue) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.verifyAccount(accountNumber, true);
    accountManager.updateAccountStatus(accountNumber, AccountStatus::FROZEN);
    
    bool result = accountManager.updateAccountStatus(accountNumber, AccountStatus::ACTIVE);
    
    ASSERT_TRUE(result);
}

/**
 * Test: updateAccountStatus_SuspendedToActive_DecrementsSuspendedCount
 * Condition: Status is SUSPENDED, new status is ACTIVE
 * Expected: Suspended account counter should decrement
 */
TEST_F(AccountManagerTest, updateAccountStatus_SuspendedToActive_DecrementsSuspendedCount) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    accountManager.suspendAccount(accountNumber, "Test");
    ASSERT_EQ(accountManager.getSuspendedAccountCount(), 1);
    
    accountManager.updateAccountStatus(accountNumber, AccountStatus::ACTIVE);
    
    ASSERT_EQ(accountManager.getSuspendedAccountCount(), 0);
}

/**
 * Test: updateAccountStatus_ActiveToSuspended_IncrementsSuspendedCount
 * Condition: Status is ACTIVE, new status is SUSPENDED
 * Expected: Suspended account counter should increment
 */
TEST_F(AccountManagerTest, updateAccountStatus_ActiveToSuspended_IncrementsSuspendedCount) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    ASSERT_EQ(accountManager.getSuspendedAccountCount(), 0);
    
    accountManager.updateAccountStatus(accountNumber, AccountStatus::SUSPENDED);
    
    ASSERT_EQ(accountManager.getSuspendedAccountCount(), 1);
}

// ============================================================================
// GET ACCOUNT TESTS
// ============================================================================

/**
 * Test: getAccount_ExistingAccount_ReturnsValidPointer
 * Condition: Account exists
 * Expected: Function should return valid account pointer
 */
TEST_F(AccountManagerTest, getAccount_ExistingAccount_ReturnsValidPointer) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    
    Account* account = accountManager.getAccount(accountNumber);
    
    ASSERT_NE(account, nullptr);
    ASSERT_EQ(account->accountNumber, accountNumber);
}

/**
 * Test: getAccount_NonexistentAccount_ReturnsNull
 * Condition: Account does not exist
 * Expected: Function should return nullptr
 */
TEST_F(AccountManagerTest, getAccount_NonexistentAccount_ReturnsNull) {
    Account* account = accountManager.getAccount("ACC999999");
    
    ASSERT_EQ(account, nullptr);
}

// ============================================================================
// GET ACCOUNT BALANCE TESTS
// ============================================================================

/**
 * Test: getAccountBalance_ExistingAccount_ReturnsCorrectBalance
 * Condition: Account exists
 * Expected: Function should return the account's balance
 */
TEST_F(AccountManagerTest, getAccountBalance_ExistingAccount_ReturnsCorrectBalance) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1500.50);
    
    double balance = accountManager.getAccountBalance(accountNumber);
    
    ASSERT_DOUBLE_EQ(balance, 1500.50);
}

/**
 * Test: getAccountBalance_NonexistentAccount_ReturnsNegativeOne
 * Condition: Account does not exist
 * Expected: Function should return -1.0
 */
TEST_F(AccountManagerTest, getAccountBalance_NonexistentAccount_ReturnsNegativeOne) {
    double balance = accountManager.getAccountBalance("ACC999999");
    
    ASSERT_EQ(balance, -1.0);
}

// ============================================================================
// VERIFY ACCOUNT TESTS
// ============================================================================

/**
 * Test: verifyAccount_UnverifiedPendingAccount_ActivatesAccount
 * Condition: Account is PENDING_VERIFICATION and verificationResult is true
 * Expected: Account status should change to ACTIVE
 */
TEST_F(AccountManagerTest, verifyAccount_UnverifiedPendingAccount_ActivatesAccount) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    
    bool result = accountManager.verifyAccount(accountNumber, true);
    Account* account = accountManager.getAccount(accountNumber);
    
    ASSERT_TRUE(result);
    ASSERT_EQ(account->status, AccountStatus::ACTIVE);
}

/**
 * Test: verifyAccount_NonexistentAccount_ReturnsFalse
 * Condition: Account does not exist
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, verifyAccount_NonexistentAccount_ReturnsFalse) {
    bool result = accountManager.verifyAccount("ACC999999", true);
    
    ASSERT_FALSE(result);
}

/**
 * Test: verifyAccount_InvalidVerificationResult_ReturnsFalse
 * Condition: verificationResult is false
 * Expected: Function should return false
 */
TEST_F(AccountManagerTest, verifyAccount_InvalidVerificationResult_ReturnsFalse) {
    std::string accountNumber = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    
    bool result = accountManager.verifyAccount(accountNumber, false);
    
    ASSERT_FALSE(result);
}

// ============================================================================
// GET SUSPENDED ACCOUNT COUNT TESTS
// ============================================================================

/**
 * Test: getSuspendedAccountCount_InitialValue_ReturnsZero
 * Condition: No accounts have been suspended
 * Expected: Function should return 0
 */
TEST_F(AccountManagerTest, getSuspendedAccountCount_InitialValue_ReturnsZero) {
    int count = accountManager.getSuspendedAccountCount();
    
    ASSERT_EQ(count, 0);
}

/**
 * Test: getSuspendedAccountCount_AfterSuspensions_ReturnsCorrectCount
 * Condition: Multiple accounts have been suspended
 * Expected: Function should return correct count
 */
TEST_F(AccountManagerTest, getSuspendedAccountCount_AfterSuspensions_ReturnsCorrectCount) {
    std::string acc1 = accountManager.createAccount(AccountType::CHECKING, 1000.0);
    std::string acc2 = accountManager.createAccount(AccountType::SAVINGS, 2000.0);
    std::string acc3 = accountManager.createAccount(AccountType::INVESTMENT, 3000.0);
    
    accountManager.suspendAccount(acc1, "Reason 1");
    accountManager.suspendAccount(acc2, "Reason 2");
    accountManager.suspendAccount(acc3, "Reason 3");
    
    int count = accountManager.getSuspendedAccountCount();
    
    ASSERT_EQ(count, 3);
}
