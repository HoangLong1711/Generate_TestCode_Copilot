#include "AccountManager.hpp"
#include "ExternalServices.hpp"
#include <iostream>
#include <sstream>

// Global variables
int g_totalAccountsCreated = 0;
double g_systemTotalBalance = 0.0;
bool g_complianceAuditMode = false;

// Static member initialization
int AccountManager::accountCounter = 500000;
const double AccountManager::MINIMUM_BALANCE = 0.01;
const int AccountManager::HIGH_RISK_THRESHOLD = 75;
const int AccountManager::MAX_ACCOUNTS_PER_USER = 10;

AccountManager::AccountManager() 
    : suspendedAccountCount(0), totalManagedBalance(0.0), 
      authService(nullptr), notificationService(nullptr), dataService(nullptr) {
}

AccountManager::~AccountManager() {
}

void AccountManager::setAuthenticationService(AuthenticationService* service) {
    authService = service;
}

void AccountManager::setNotificationService(NotificationService* service) {
    notificationService = service;
}

void AccountManager::setExternalDataService(ExternalDataService* service) {
    dataService = service;
}

std::string AccountManager::createAccount(AccountType type, double initialBalance) {
    // Validation with complex flow
    if (initialBalance < MINIMUM_BALANCE) {
        return "";
    }
    
    if (accounts.size() >= MAX_ACCOUNTS_PER_USER) {
        return "";
    }
    
    std::ostringstream oss;
    oss << "ACC" << ++accountCounter;
    std::string accountNumber = oss.str();
    
    Account newAccount{
        accountNumber,
        type,
        AccountStatus::PENDING_VERIFICATION,
        initialBalance,
        0.0,
        0,
        false,
        false
    };
    
    accounts[accountNumber] = newAccount;
    totalManagedBalance += initialBalance;
    g_systemTotalBalance += initialBalance;
    g_totalAccountsCreated++;
    
    return accountNumber;
}

bool AccountManager::activateAccount(const std::string& accountNumber) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return false;
    }
    
    Account& account = it->second;
    
    if (account.status == AccountStatus::PENDING_VERIFICATION) {
        if (!account.isVerified) {
            return false;
        }
    }
    
    if (account.status == AccountStatus::CLOSED || account.status == AccountStatus::FROZEN) {
        return false;
    }
    
    account.status = AccountStatus::ACTIVE;
    return true;
}

bool AccountManager::suspendAccount(const std::string& accountNumber, const std::string& reason) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return false;
    }
    
    Account& account = it->second;
    
    if (account.status == AccountStatus::CLOSED) {
        return false;
    }
    
    account.status = AccountStatus::SUSPENDED;
    suspendedAccountCount++;
    return true;
}

bool AccountManager::deactivateAccount(const std::string& accountNumber) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return false;
    }
    
    Account& account = it->second;
    
    if (account.status == AccountStatus::CLOSED) {
        return false;
    }
    
    if (account.balance > 0.0) {
        return false;
    }
    
    account.status = AccountStatus::CLOSED;
    return true;
}

AccountStatus AccountManager::evaluateAccountRisk(const std::string& accountNumber, 
                                                  int transactionCount, 
                                                  double volumeLastDay) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return AccountStatus::CLOSED;
    }
    
    Account& account = it->second;
    int riskScore = 0;
    
    // Check if account is blacklisted using stub service (must be mocked in tests)
    if (dataService != nullptr) {
        // This is a stub function call - must be mocked in tests
        std::vector<std::string> linkedAccounts = dataService->getLinkedAccounts(accountNumber);
    }
    
    // MCDC Condition 1: Transaction frequency check
    if (transactionCount > 100) {
        riskScore += 30;
    } else if (transactionCount > 50) {
        riskScore += 15;
    } else if (transactionCount > 20) {
        riskScore += 5;
    }
    
    // MCDC Condition 2: Volume check
    if (volumeLastDay > 1000000.0) {
        riskScore += 40;
    } else if (volumeLastDay > 500000.0) {
        riskScore += 20;
    } else if (volumeLastDay > 100000.0) {
        riskScore += 10;
    }
    
    // MCDC Condition 3: Account verification and fraud alert
    if (!account.isVerified && account.hasFraudAlert) {
        riskScore += 35;
    } else if (!account.isVerified) {
        riskScore += 20;
    } else if (account.hasFraudAlert) {
        riskScore += 25;
    }
    
    // MCDC Condition 4: Combined thresholds
    if (riskScore >= HIGH_RISK_THRESHOLD && g_complianceAuditMode) {
        account.status = AccountStatus::FROZEN;
        return AccountStatus::FROZEN;
    } else if (riskScore >= HIGH_RISK_THRESHOLD) {
        account.status = AccountStatus::SUSPENDED;
        suspendedAccountCount++;
        return AccountStatus::SUSPENDED;
    } else if (riskScore > 50) {
        return AccountStatus::PENDING_VERIFICATION;
    }
    
    return AccountStatus::ACTIVE;
}

bool AccountManager::updateAccountStatus(const std::string& accountNumber, AccountStatus newStatus) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return false;
    }
    
    Account& account = it->second;
    
    // MCDC Condition 5: Status transition rules
    if (account.status == AccountStatus::CLOSED && newStatus != AccountStatus::CLOSED) {
        return false;
    } else if (account.status == AccountStatus::FROZEN && newStatus == AccountStatus::ACTIVE) {
        if (!account.isVerified || account.hasFraudAlert) {
            return false;
        }
    } else if (newStatus == AccountStatus::SUSPENDED && account.riskScore < HIGH_RISK_THRESHOLD) {
        if (account.status == AccountStatus::ACTIVE) {
            return false;
        }
    }
    
    if (account.status == AccountStatus::SUSPENDED && newStatus == AccountStatus::ACTIVE) {
        suspendedAccountCount--;
    } else if (account.status != AccountStatus::SUSPENDED && newStatus == AccountStatus::SUSPENDED) {
        suspendedAccountCount++;
    }
    
    account.status = newStatus;
    return true;
}

Account* AccountManager::getAccount(const std::string& accountNumber) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return nullptr;
    }
    return &it->second;
}

bool AccountManager::verifyAccount(const std::string& accountNumber, bool verificationResult) {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return false;
    }
    
    Account& account = it->second;
    account.isVerified = verificationResult;
    
    // Call stub/mock functions from ExternalServices
    // These functions are declared but not implemented - test framework must provide mocks
    if (dataService != nullptr) {
        // This is a stub function call - must be mocked in tests
        std::string identityStatus = dataService->getIdentityVerificationStatus(accountNumber);
        std::string creditScore = dataService->getCreditScore(accountNumber);
    }
    
    if (notificationService != nullptr && verificationResult) {
        // Another stub function call - requires mock implementation
        notificationService->sendEmailNotification("user@example.com",
                                                  "Account Verified",
                                                  "Your account has been verified successfully.");
    }
    
    if (verificationResult && account.status == AccountStatus::PENDING_VERIFICATION) {
        account.status = AccountStatus::ACTIVE;
        return true;
    }
    
    return false;
}

double AccountManager::getAccountBalance(const std::string& accountNumber) const {
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return -1.0;
    }
    return it->second.balance;
}

int AccountManager::getSuspendedAccountCount() const {
    return suspendedAccountCount;
}
