#ifndef BOOT_MANAGER_H
#define BOOT_MANAGER_H

#include <functional>
#include <vector>
#include <string>
#include "infrastructure/logger.h"

/**
 * BootManager - Manages the system bootup sequence
 *
 * Provides a structured approach to system initialization with:
 * - Boot phases (hardware, services, application)
 * - Dependency management
 * - Error recovery and rollback
 * - Progress tracking
 * - Graceful degradation for non-critical components
 *
 * Similar to init systems in Linux (systemd) or Android init.rc
 */
class BootManager {
public:
    // Boot phases - similar to Linux runlevels or Android init stages
    enum class BootPhase {
        PRE_INIT,      // Logger, basic hardware
        HARDWARE_INIT, // Core hardware (display, audio)
        NETWORK_INIT,  // WiFi, network services
        SERVICE_INIT,  // Application services (GPT, voice, etc.)
        APPLICATION_START, // Start main application logic
        READY          // System fully operational
    };

    // Component initialization result
    struct InitResult {
        bool success;
        std::string componentName;
        std::string errorMessage;
        bool critical; // If true, failure stops boot process
    };

    // Component initializer function type
    using ComponentInitializer = std::function<InitResult()>;

    /**
     * Get singleton instance
     */
    static BootManager& getInstance();

    /**
     * Initialize the boot manager
     */
    bool init();

    /**
     * Execute the complete boot sequence
     */
    bool bootSystem();

    /**
     * Register a component for a specific boot phase
     */
    void registerComponent(BootPhase phase, ComponentInitializer initializer,
                          const std::string& name, bool critical = true);

    /**
     * Get current boot phase
     */
    BootPhase getCurrentPhase() const { return currentPhase; }

    /**
     * Get boot progress (0.0 to 1.0)
     */
    float getBootProgress() const;

    /**
     * Check if system is fully booted
     */
    bool isSystemReady() const { return currentPhase == BootPhase::READY; }

    /**
     * Get initialization results for debugging
     */
    const std::vector<InitResult>& getInitResults() const { return initResults; }

private:
    BootManager() = default;
    BootManager(const BootManager&) = delete;
    BootManager& operator=(const BootManager&) = delete;

    struct BootComponent {
        BootPhase phase;
        ComponentInitializer initializer;
        std::string name;
        bool critical;
    };

    BootPhase currentPhase = BootPhase::PRE_INIT;
    std::vector<BootComponent> components;
    std::vector<InitResult> initResults;
    bool initialized = false;

    // Phase execution methods
    InitResult executePhase(BootPhase phase);
    std::string phaseToString(BootPhase phase) const;
    bool shouldContinueAfterFailure(const InitResult& result) const;
};

#endif // BOOT_MANAGER_H