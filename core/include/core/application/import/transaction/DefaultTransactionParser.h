/**
 * @file core/include/core/application/import/transaction/DefaultTransactionParser.h
 * @brief Declares the default transaction parser used by the import pipeline.
 */

#pragma once

#include "TransactionParserTypes.h"
#include "core/application/import/transaction/AmountParser.h"

#include <string>

namespace core::application::importing::transaction {
using internal::TransactionBlock;

/**
 * @brief Represents the parsed fields of a single transaction.
 */
struct DefaultTransactionParser {
    std::string name;
    std::string bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string description;
    std::string metadata;
    int status = 0;

    /**
     * @brief Parses a transaction block into a structured transaction model.
     * @param block Transaction block to parse.
     * @param debugOut Optional debug output sink.
     * @return Parsed transaction model.
     */
    static DefaultTransactionParser parseTransaction(const TransactionBlock& block,
                                                     std::vector<std::string>* debugOut = nullptr);
};

}
