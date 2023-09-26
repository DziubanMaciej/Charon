#include "oakum/oakum_api.h"

#include <gtest/gtest.h>

#define EXPECT_OAKUM_SUCCESS(expr) EXPECT_EQ(OAKUM_SUCCESS, (expr))

class OakumGtestEventListener : public ::testing::EmptyTestEventListener {
public:
    OakumGtestEventListener(bool captureStackTraces)
        : captureStackTraces(captureStackTraces) {}

    void OnTestStart([[maybe_unused]] const ::testing::TestInfo &test_info) override {
        OakumInitArgs args{};
        args.threadSafe = true;
        args.sortAllocations = true;
        if (captureStackTraces) {
            args.trackStackTraces = true;
            args.fallbackSourceFileName = "<unknown_file>";
            args.fallbackSymbolName = "<unknown_symbol>";
        }
        EXPECT_OAKUM_SUCCESS(oakumInit(&args));
    }

    void OnTestEnd([[maybe_unused]] const ::testing::TestInfo &test_info) override {
        if (oakumDetectLeaks() != OAKUM_SUCCESS) {
            GTEST_NONFATAL_FAILURE_("Memory leaks detected!");
            if (captureStackTraces) {
                OakumAllocation *allocations{};
                size_t allocationsCount{};
                EXPECT_OAKUM_SUCCESS(oakumGetAllocations(&allocations, &allocationsCount));
                EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSymbols(allocations, 1));
                EXPECT_OAKUM_SUCCESS(oakumResolveStackTraceSourceLocations(allocations, 1));
                std::cout << "Total leaks found: " << allocationsCount << ". Showing the first one:\n";
                std::cout << "  id=" << allocations[0].allocationId << ", size=" << allocations[0].size << '\n';
                for (size_t stackFrameIndex = 0u; stackFrameIndex < allocations[0].stackFramesCount; stackFrameIndex++) {
                    OakumStackFrame &frame = allocations[0].stackFrames[stackFrameIndex];
                    std::cout << "    " << stackFrameIndex << ". " << frame.symbolName << " in file " << frame.fileName << ":" << frame.fileLine << "\n";
                }
                EXPECT_OAKUM_SUCCESS(oakumReleaseAllocations(allocations, allocationsCount));
            }
        }

        EXPECT_OAKUM_SUCCESS(oakumDeinit(false));
    }

private:
    const bool captureStackTraces;
};
