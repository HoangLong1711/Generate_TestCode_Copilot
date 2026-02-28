#ifndef TRANSACTION_PROCESSOR_HPP
#define TRANSACTION_PROCESSOR_HPP

#include <string>
#include <vector>
#include <ctime>

class ComplianceCheckService;
class AuditLoggingService;

enum class TransactionStatus {
    PENDING,
    APPROVED,
    REJECTED,
    CANCELLED,
    COMPLETED
};

enum class TransactionType {
    DEPOSIT,
    WITHDRAWAL,
    TRANSFER,
    REFUND
};

struct Transaction {
    int id;
    TransactionType type;
    double amount;
    std::string sourceAccount;
    std::string destAccount;
    time_t timestamp;
    TransactionStatus status;
};

class TransactionProcessor {
private:
    static int transactionCounter;
    static const double MIN_TRANSACTION_AMOUNT;
    static const double MAX_TRANSACTION_AMOUNT;
    static const int MAX_DAILY_TRANSACTIONS;
    
    std::vector<Transaction> transactionHistory;
    double dailyVolume;
    int dailyTransactionCount;
    
    // External service pointers (stub/mock for testing)
    ComplianceCheckService* complianceService;
    AuditLoggingService* auditService;

public:
    /// @brief Constructs a TransactionProcessor instance.
    /// @details Initializes the transaction processor with empty history and zero counters.
    TransactionProcessor();
    
    /// @brief Destructs the TransactionProcessor instance.
    ~TransactionProcessor();
    
    /// @brief Sets the compliance service for transaction validation.
    /// @param [in] service Pointer to the ComplianceCheckService implementation.
    void setComplianceService(ComplianceCheckService* service);
    
    /// @brief Sets the audit service for logging transactions.
    /// @param [in] service Pointer to the AuditLoggingService implementation.
    void setAuditService(AuditLoggingService* service);
    
    /// @brief Processes a transaction with the specified parameters.
    /// @param [in] type The type of transaction to process.
    /// @param [in] amount The transaction amount.
    /// @param [in] sourceAccount The source account number.
    /// @param [in] destAccount The destination account number.
    /// @return The status of the processed transaction.
    TransactionStatus processTransaction(TransactionType type, 
                                        double amount, 
                                        const std::string& sourceAccount,
                                        const std::string& destAccount);
    
    /// @brief Validates a transaction amount and type.
    /// @param [in] amount The transaction amount to validate.
    /// @param [in] type The transaction type to validate.
    /// @return True if the transaction is valid, false otherwise.
    bool validateTransaction(double amount, TransactionType type);
    
    /// @brief Executes a fund transfer between accounts.
    /// @param [in] amount The amount to transfer.
    /// @param [in] source The source account number.
    /// @param [in] destination The destination account number.
    /// @param [in] isUrgent Whether the transfer is marked as urgent.
    /// @return The status of the transfer.
    TransactionStatus executeTransfer(double amount, 
                                     const std::string& source,
                                     const std::string& destination,
                                     bool isUrgent);
    
    /// @brief Logs a transaction to the history.
    /// @param [in] transaction The transaction to log.
    void logTransaction(const Transaction& transaction);
    
    /// @brief Resets daily transaction limits and counters.
    /// @return True if the reset was successful, false otherwise.
    bool resetDailyLimits();
    
    /// @brief Retrieves the current daily transaction volume.
    /// @return The total daily transaction volume.
    double getDailyVolume() const;
    
    /// @brief Retrieves the number of transactions completed today.
    /// @return The count of daily transactions.
    int getTransactionCount() const;
};

#endif // TRANSACTION_PROCESSOR_HPP
