#ifndef EXTERNAL_SERVICES_HPP
#define EXTERNAL_SERVICES_HPP

#include <string>
#include <vector>

enum class VerificationResult {
    SUCCESS,
    FAILED,
    PENDING,
    TIMEOUT,
    NETWORK_ERROR
};

enum class ComplianceLevel {
    LOW_RISK,
    MEDIUM_RISK,
    HIGH_RISK,
    BLOCKED
};

class AuthenticationService {
public:
    /// @brief Destructor for AuthenticationService.
    virtual ~AuthenticationService() = default;
    
    /// @brief Validates user credentials.
    /// @param [in] username The username to validate.
    /// @param [in] password The password to validate.
    /// @return True if credentials are valid, false otherwise.
    virtual bool validateCredentials(const std::string& username, const std::string& password) = 0;
    
    /// @brief Enables multi-factor authentication for an account.
    /// @param [in] accountNumber The account number to enable MFA for.
    /// @return True if MFA was successfully enabled, false otherwise.
    virtual bool enableMultiFactor(const std::string& accountNumber) = 0;
    
    /// @brief Verifies a multi-factor authentication token.
    /// @param [in] accountNumber The account number to verify.
    /// @param [in] token The authentication token to verify.
    /// @return The verification result status.
    virtual VerificationResult verifyMultiFactorToken(const std::string& accountNumber, 
                                                      const std::string& token) = 0;
    
    /// @brief Locks an account for security purposes.
    /// @param [in] accountNumber The account number to lock.
    /// @return True if the account was successfully locked, false otherwise.
    virtual bool lockAccount(const std::string& accountNumber) = 0;
};

class ComplianceCheckService {
public:
    /// @brief Destructor for ComplianceCheckService.
    virtual ~ComplianceCheckService() = default;
    
    /// @brief Checks the compliance level of an account.
    /// @param [in] accountNumber The account number to check.
    /// @return The compliance level of the account.
    virtual ComplianceLevel checkComplianceLevel(const std::string& accountNumber) = 0;
    
    /// @brief Reports suspicious activity on an account.
    /// @param [in] accountNumber The account number with suspicious activity.
    /// @param [in] description The description of the suspicious activity.
    /// @return True if the report was recorded successfully, false otherwise.
    virtual bool reportSuspiciousActivity(const std::string& accountNumber, 
                                         const std::string& description) = 0;
    
    /// @brief Retrieves the account blacklist.
    /// @return A vector of blacklisted account numbers.
    virtual std::vector<std::string> getBlacklist() = 0;
    
    /// @brief Checks if an account is blacklisted.
    /// @param [in] accountNumber The account number to check.
    /// @return True if the account is blacklisted, false otherwise.
    virtual bool isAccountBlacklisted(const std::string& accountNumber) = 0;
};

class AuditLoggingService {
public:
    /// @brief Destructor for AuditLoggingService.
    virtual ~AuditLoggingService() = default;
    
    /// @brief Logs a transaction event.
    /// @param [in] accountNumber The account number involved in the transaction.
    /// @param [in] transactionDetails Details of the transaction.
    /// @param [in] timestamp The timestamp of the transaction.
    /// @return True if the log was recorded successfully, false otherwise.
    virtual bool logTransaction(const std::string& accountNumber, 
                               const std::string& transactionDetails,
                               const std::string& timestamp) = 0;
    
    /// @brief Logs an account event.
    /// @param [in] accountNumber The account number associated with the event.
    /// @param [in] eventType The type of event.
    /// @param [in] eventDetails Details of the event.
    /// @return True if the log was recorded successfully, false otherwise.
    virtual bool logAccountEvent(const std::string& accountNumber, 
                                const std::string& eventType,
                                const std::string& eventDetails) = 0;
    
    /// @brief Retrieves the audit trail for an account.
    /// @param [in] accountNumber The account number.
    /// @return A vector of audit log entries for the account.
    virtual std::vector<std::string> getAuditTrail(const std::string& accountNumber) = 0;
    
    /// @brief Archives audit logs.
    /// @param [in] archiveDate The date up to which logs should be archived.
    /// @return True if the archive operation was successful, false otherwise.
    virtual bool archiveAuditLogs(const std::string& archiveDate) = 0;
};

class NotificationService {
public:
    /// @brief Destructor for NotificationService.
    virtual ~NotificationService() = default;
    
    /// @brief Sends an email notification.
    /// @param [in] email The recipient email address.
    /// @param [in] subject The email subject line.
    /// @param [in] body The email body content.
    /// @return True if the email was sent successfully, false otherwise.
    virtual bool sendEmailNotification(const std::string& email, 
                                      const std::string& subject,
                                      const std::string& body) = 0;
    
    /// @brief Sends an SMS notification.
    /// @param [in] phoneNumber The recipient phone number.
    /// @param [in] message The SMS message content.
    /// @return True if the SMS was sent successfully, false otherwise.
    virtual bool sendSmsNotification(const std::string& phoneNumber, 
                                    const std::string& message) = 0;
    
    /// @brief Sends a push notification.
    /// @param [in] deviceToken The device token for the notification.
    /// @param [in] title The notification title.
    /// @param [in] message The notification message content.
    /// @return True if the notification was sent successfully, false otherwise.
    virtual bool sendPushNotification(const std::string& deviceToken, 
                                     const std::string& title,
                                     const std::string& message) = 0;
    
    /// @brief Subscribes an account to notifications.
    /// @param [in] accountNumber The account number to subscribe.
    /// @param [in] notificationType The type of notifications to subscribe to.
    /// @return True if the subscription was successful, false otherwise.
    virtual bool subscribeToNotifications(const std::string& accountNumber, 
                                         const std::string& notificationType) = 0;
};

class ExternalDataService {
public:
    /// @brief Destructor for ExternalDataService.
    virtual ~ExternalDataService() = default;
    
    /// @brief Retrieves the credit score for an account.
    /// @param [in] accountNumber The account number.
    /// @return The credit score as a string.
    virtual std::string getCreditScore(const std::string& accountNumber) = 0;
    
    /// @brief Gets the identity verification status for an account.
    /// @param [in] accountNumber The account number.
    /// @return The identity verification status as a string.
    virtual std::string getIdentityVerificationStatus(const std::string& accountNumber) = 0;
    
    /// @brief Validates a bank account with routing information.
    /// @param [in] bankAccount The bank account number to validate.
    /// @param [in] routingNumber The routing number associated with the account.
    /// @return True if the bank account is valid, false otherwise.
    virtual bool validateBankAccount(const std::string& bankAccount, 
                                    const std::string& routingNumber) = 0;
    
    /// @brief Retrieves accounts linked to a primary account.
    /// @param [in] primaryAccount The primary account number.
    /// @return A vector of linked account numbers.
    virtual std::vector<std::string> getLinkedAccounts(const std::string& primaryAccount) = 0;
};

class RateLimitingService {
public:
    /// @brief Destructor for RateLimitingService.
    virtual ~RateLimitingService() = default;
    
    /// @brief Checks if an account has exceeded its rate limit.
    /// @param [in] accountNumber The account number to check.
    /// @return True if the account is within rate limits, false if exceeded.
    virtual bool checkRateLimit(const std::string& accountNumber) = 0;
    
    /// @brief Increments the rate counter for an account.
    /// @param [in] accountNumber The account number to increment.
    /// @return True if the counter was incremented successfully, false otherwise.
    virtual bool incrementRateCounter(const std::string& accountNumber) = 0;
    
    /// @brief Resets rate limits for an account.
    /// @param [in] accountNumber The account number to reset.
    virtual void resetRateLimits(const std::string& accountNumber) = 0;
    
    /// @brief Retrieves the remaining requests for an account.
    /// @param [in] accountNumber The account number.
    /// @return The number of remaining requests allowed.
    virtual int getRemainingRequests(const std::string& accountNumber) = 0;
};

#endif // EXTERNAL_SERVICES_HPP
