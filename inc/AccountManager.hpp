#ifndef ACCOUNT_MANAGER_HPP
#define ACCOUNT_MANAGER_HPP

#include <string>
#include <map>
#include <vector>

class AuthenticationService;
class NotificationService;
class ExternalDataService;

enum class AccountStatus {
    ACTIVE,
    SUSPENDED,
    FROZEN,
    CLOSED,
    PENDING_VERIFICATION
};

enum class AccountType {
    CHECKING,
    SAVINGS,
    INVESTMENT,
    BUSINESS
};

struct Account {
    std::string accountNumber;
    AccountType type;
    AccountStatus status;
    double balance;
    double creditLimit;
    int riskScore;
    bool isVerified;
    bool hasFraudAlert;
};

class AccountManager {
private:
    static int accountCounter;
    static const double MINIMUM_BALANCE;
    static const int HIGH_RISK_THRESHOLD;
    static const int MAX_ACCOUNTS_PER_USER;
    
    std::map<std::string, Account> accounts;
    int suspendedAccountCount;
    double totalManagedBalance;
    
    // External service pointers (stub/mock for testing)
    AuthenticationService* authService;
    NotificationService* notificationService;
    ExternalDataService* dataService;

public:
    /// @brief Constructs an AccountManager instance.
    /// @details Initializes the account manager with empty account storage and zero counters.
    AccountManager();
    
    /// @brief Destructs the AccountManager instance.
    ~AccountManager();
    
    /// @brief Sets the authentication service for account operations.
    /// @param [in] service Pointer to the AuthenticationService implementation.
    void setAuthenticationService(AuthenticationService* service);
    
    /// @brief Sets the notification service for sending account notifications.
    /// @param [in] service Pointer to the NotificationService implementation.
    void setNotificationService(NotificationService* service);
    
    /// @brief Sets the external data service for account verification.
    /// @param [in] service Pointer to the ExternalDataService implementation.
    void setExternalDataService(ExternalDataService* service);
    
    /// @brief Creates a new account with the specified type and initial balance.
    /// @param [in] type The type of account to create.
    /// @param [in] initialBalance The initial balance for the account.
    /// @return A unique account number as a string.
    std::string createAccount(AccountType type, double initialBalance);
    
    /// @brief Activates a suspended or inactive account.
    /// @param [in] accountNumber The account number to activate.
    /// @return True if activation succeeded, false otherwise.
    bool activateAccount(const std::string& accountNumber);
    
    /// @brief Suspends an account with a specified reason.
    /// @param [in] accountNumber The account number to suspend.
    /// @param [in] reason The reason for suspension.
    /// @return True if suspension succeeded, false otherwise.
    bool suspendAccount(const std::string& accountNumber, const std::string& reason);
    
    /// @brief Deactivates an account.
    /// @param [in] accountNumber The account number to deactivate.
    /// @return True if deactivation succeeded, false otherwise.
    bool deactivateAccount(const std::string& accountNumber);
    
    /// @brief Evaluates the risk level of an account based on transaction activity.
    /// @param [in] accountNumber The account number to evaluate.
    /// @param [in] transactionCount The number of transactions in the evaluation period.
    /// @param [in] volumeLastDay The transaction volume from the last day.
    /// @return The evaluated account status based on risk assessment.
    AccountStatus evaluateAccountRisk(const std::string& accountNumber, 
                                      int transactionCount, 
                                      double volumeLastDay);
    
    /// @brief Updates the status of an account.
    /// @param [in] accountNumber The account number to update.
    /// @param [in] newStatus The new account status.
    /// @return True if the status update succeeded, false otherwise.
    bool updateAccountStatus(const std::string& accountNumber, AccountStatus newStatus);
    
    /// @brief Retrieves the account information.
    /// @param [in] accountNumber The account number to retrieve.
    /// @return Pointer to the Account object, or nullptr if not found.
    Account* getAccount(const std::string& accountNumber);
    
    /// @brief Verifies or updates the verification status of an account.
    /// @param [in] accountNumber The account number to verify.
    /// @param [in] verificationResult The verification result.
    /// @return True if verification update succeeded, false otherwise.
    bool verifyAccount(const std::string& accountNumber, bool verificationResult);
    
    /// @brief Retrieves the current balance of an account.
    /// @param [in] accountNumber The account number.
    /// @return The account balance as a double.
    double getAccountBalance(const std::string& accountNumber) const;
    
    /// @brief Retrieves the count of currently suspended accounts.
    /// @return The number of suspended accounts.
    int getSuspendedAccountCount() const;
};

#endif // ACCOUNT_MANAGER_HPP
