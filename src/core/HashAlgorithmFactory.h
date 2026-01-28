#ifndef HASH_ALGORITHM_FACTORY_H
#define HASH_ALGORITHM_FACTORY_H

#include "IHashAlgorithm.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace core {

/**
 * @brief Factory for creating hash algorithm instances.
 * 
 * Algorithms self-register using HashAlgorithmRegistrar.
 * 
 * Usage:
 *   auto sha256 = HashAlgorithmFactory::create("SHA-256");
 *   auto algos = HashAlgorithmFactory::getAvailableAlgorithms();
 */
class HashAlgorithmFactory {
public:
    using Creator = std::function<std::unique_ptr<IHashAlgorithm>()>;

    /// @brief Create algorithm by name
    /// @throws std::invalid_argument if algorithm not found
    static std::unique_ptr<IHashAlgorithm> create(const std::string& name);

    /// @brief Get list of available algorithm names
    static std::vector<std::string> getAvailableAlgorithms();

    /// @brief Check if algorithm is available
    static bool isAvailable(const std::string& name);

    /// @brief Register algorithm creator (used by registrar)
    static void registerAlgorithm(const std::string& name, Creator creator);

private:
    static std::map<std::string, Creator>& getRegistry();
};

/**
 * @brief RAII helper for auto-registration of hash algorithms.
 * 
 * Usage (in .cpp file):
 *   static HashAlgorithmRegistrar<SHA256Hash> registrar("SHA-256");
 */
template<typename T>
class HashAlgorithmRegistrar {
public:
    explicit HashAlgorithmRegistrar(const std::string& name) {
        HashAlgorithmFactory::registerAlgorithm(name, []() {
            return std::make_unique<T>();
        });
    }
};

} // namespace core

#endif // HASH_ALGORITHM_FACTORY_H
