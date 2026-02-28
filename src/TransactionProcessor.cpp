#include "TransactionProcessor.hpp"
#include "ExternalServices.hpp"
#include <iostream>
#include <cmath>

// Global variables
int g_totalTransactionsProcessed = 0;
double g_totalVolumeProcessed = 0.0;
bool g_systemLocked = false;

// Static member initialization
int TransactionProcessor::transactionCounter = 1000;
const double TransactionProcessor::MIN_TRANSACTION_AMOUNT = 0.01;
const double TransactionProcessor::MAX_TRANSACTION_AMOUNT = 1000000.0;
const int TransactionProcessor::MAX_DAILY_TRANSACTIONS = 1000;

TransactionProcessor::TransactionProcessor() 
    : dailyVolume(0.0), dailyTransactionCount(0), complianceService(nullptr), auditService(nullptr) {
}

TransactionProcessor::~TransactionProcessor() {
}

void TransactionProcessor::setComplianceService(ComplianceCheckService* service) {
    complianceService = service;
}

void TransactionProcessor::setAuditService(AuditLoggingService* service) {
    auditService = service;
}

bool TransactionProcessor::validateTransaction(double amount, TransactionType type) {
    // Complex MCDC condition 1: Amount and Type validation
    if (amount < MIN_TRANSACTION_AMOUNT) {
        return false;
    } else if (amount > MAX_TRANSACTION_AMOUNT) {
        return false;
    } else if (type == TransactionType::WITHDRAWAL && amount > 50000.0) {
        // Special rule for large withdrawals
        return false;
    } else if (type == TransactionType::REFUND && amount > 10000.0) {
        // Refunds have lower limits
        return false;
    } else {
        return true;
    }
}

TransactionStatus TransactionProcessor::executeTransfer(double amount, 
                                                        const std::string& source,
                                                        const std::string& destination,
                                                        bool isUrgent) {
    // Complex MCDC condition 2-5: Multi-condition transfer logic
    if (source.empty() || destination.empty()) {
        return TransactionStatus::REJECTED;
    }
    
    // Condition 2: Check if accounts are same
    if (source == destination) {
        if (amount > 0.0) {
            return TransactionStatus::REJECTED;
        } else {
            return TransactionStatus::CANCELLED;
        }
    }
    
    // Condition 3: Urgent transfer rules
    if (isUrgent && amount > 100000.0) {
        if (dailyTransactionCount >= MAX_DAILY_TRANSACTIONS) {
            return TransactionStatus::REJECTED;
        } else if (dailyVolume + amount > 5000000.0) {
            return TransactionStatus::REJECTED;
        }
    }
    
    // Condition 4: System lock check
    if (g_systemLocked && !isUrgent) {
        return TransactionStatus::PENDING;
    } else if (g_systemLocked && isUrgent) {
        return TransactionStatus::APPROVED;
    }
    
    // Condition 5: Final validation before execution
    if (amount > 0.0 && 
        dailyTransactionCount < MAX_DAILY_TRANSACTIONS && 
        dailyVolume + amount <= 5000000.0) {
        return TransactionStatus::COMPLETED;
    } else if (amount > 0.0 && dailyTransactionCount < MAX_DAILY_TRANSACTIONS) {
        return TransactionStatus::APPROVED;
    } else if (amount > 0.0) {
        return TransactionStatus::PENDING;
    } else {
        return TransactionStatus::CANCELLED;
    }
}

TransactionStatus TransactionProcessor::processTransaction(TransactionType type, 
                                                           double amount, 
                                                           const std::string& sourceAccount,
                                                           const std::string& destAccount) {
    // Validation phase
    if (!validateTransaction(amount, type)) {
        return TransactionStatus::REJECTED;
    }
    
    // Check compliance using stub service (must be mocked in tests)
    if (complianceService != nullptr) {
        ComplianceLevel complianceLevel = complianceService->checkComplianceLevel(sourceAccount);
        if (complianceLevel == ComplianceLevel::BLOCKED) {
            return TransactionStatus::REJECTED;
        }
        
        if (complianceLevel == ComplianceLevel::HIGH_RISK && amount > 50000.0) {
            return TransactionStatus::REJECTED;
        }
    }
    
    // Process based on type
    TransactionStatus status = TransactionStatus::PENDING;
    
    if (type == TransactionType::TRANSFER) {
        status = executeTransfer(amount, sourceAccount, destAccount, false);
    } else if (type == TransactionType::DEPOSIT) {
        if (amount > 0.0 && dailyTransactionCount < MAX_DAILY_TRANSACTIONS) {
            status = TransactionStatus::COMPLETED;
            dailyVolume += amount;
            g_totalVolumeProcessed += amount;
        } else {
            status = TransactionStatus::REJECTED;
        }
    } else if (type == TransactionType::WITHDRAWAL) {
        if (amount > 0.0 && amount <= 50000.0 && dailyTransactionCount < MAX_DAILY_TRANSACTIONS) {
            status = TransactionStatus::COMPLETED;
            dailyVolume += amount;
        } else if (dailyTransactionCount >= MAX_DAILY_TRANSACTIONS) {
            status = TransactionStatus::REJECTED;
        } else {
            status = TransactionStatus::PENDING;
        }
    } else if (type == TransactionType::REFUND) {
        if (amount > 0.0 && amount <= 10000.0) {
            status = TransactionStatus::COMPLETED;
        } else if (amount > 10000.0) {
            status = TransactionStatus::PENDING;
        }
    } else {
        status = TransactionStatus::CANCELLED;
    }
    
    // Log and update counters
    if (status != TransactionStatus::REJECTED && status != TransactionStatus::CANCELLED) {
        logTransaction(Transaction{
            ++transactionCounter,
            type,
            amount,
            sourceAccount,
            destAccount,
            time(nullptr),
            status
        });
        
        dailyTransactionCount++;
        g_totalTransactionsProcessed++;
    }
    
    return status;
}

void TransactionProcessor::logTransaction(const Transaction& transaction) {
    transactionHistory.push_back(transaction);
    std::cout << "Transaction ID: " << transaction.id 
              << " Status: " << static_cast<int>(transaction.status) << std::endl;
    
    // Call stub/mock functions from ExternalServices
    // These functions are declared but not implemented - test framework must provide mocks
    if (auditService != nullptr) {
        // This is a stub function call - must be mocked in tests
        auditService->logTransaction(transaction.sourceAccount, 
                                     std::to_string(transaction.amount), 
                                     std::to_string(time(nullptr)));
        
        // Another stub function call
        auditService->logAccountEvent(transaction.sourceAccount,
                                     "TRANSACTION_PROCESSED",
                                     "Transaction: " + std::to_string(transaction.id));
    }
}

bool TransactionProcessor::resetDailyLimits() {
    dailyVolume = 0.0;
    dailyTransactionCount = 0;
    return true;
}

double TransactionProcessor::getDailyVolume() const {
    return dailyVolume;
}

int TransactionProcessor::getTransactionCount() const {
    return dailyTransactionCount;
}
