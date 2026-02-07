#include "HashAlgorithmFactory.h"
#include <stdexcept>

namespace core {

std::map<std::string, HashAlgorithmFactory::Creator>& HashAlgorithmFactory::getRegistry() {
    // Use a pointer to avoid destruction order issues
    // The map is intentionally leaked to prevent crashes during static deinitialization
    static std::map<std::string, Creator>* registry = new std::map<std::string, Creator>();
    return *registry;
}

void HashAlgorithmFactory::registerAlgorithm(const std::string& name, Creator creator) {
    getRegistry()[name] = std::move(creator);
}

std::unique_ptr<IHashAlgorithm> HashAlgorithmFactory::create(const std::string& name) {
    auto& registry = getRegistry();
    auto it = registry.find(name);
    if (it == registry.end()) {
        throw std::invalid_argument("Unknown hash algorithm: " + name);
    }
    return it->second();
}

std::vector<std::string> HashAlgorithmFactory::getAvailableAlgorithms() {
    std::vector<std::string> names;
    for (const auto& [name, creator] : getRegistry()) {
        names.push_back(name);
    }
    return names;
}

bool HashAlgorithmFactory::isAvailable(const std::string& name) {
    return getRegistry().count(name) > 0;
}

} // namespace core
