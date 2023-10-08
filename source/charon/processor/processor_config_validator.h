#pragma once

#include "charon/util/class_traits.h"
#include "charon/util/filesystem.h"

struct ProcessorConfig;
struct ProcessorAction;
struct ProcessorActionMatcher;

class ProcessorConfigValidator : NonInstantiatable {
public:
    static bool validateConfig(const ProcessorConfig &config);

private:
    static bool validateActionMatcher(const ProcessorActionMatcher &actionMatcher);
    static bool validateExtension(const fs::path &extension);
    static bool validateActions(const std::vector<ProcessorAction> &actions);
    static bool validateAction(const ProcessorAction &action, bool &foundRemoveAction);
    static bool validatePath(const fs::path &path, const std::string &label, bool allowEmpty, bool allowNonExistant);
};
