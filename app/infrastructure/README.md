# Infrastructure Layer 🏗️

This folder contains the system-level infrastructure components that provide foundational services for the ESP32 voice assistant application.

## 📋 Components

### Boot Manager (`boot_manager.h/.cpp`)
**Purpose**: Manages the system bootup sequence following OS-inspired patterns with phase-based initialization.

**Key Features**:
- **Boot Phases**: PRE_INIT → HARDWARE_INIT → NETWORK_INIT → SERVICE_INIT → APPLICATION_START → READY
- **Component Registration**: Register boot components with priorities and dependencies
- **Error Handling**: Comprehensive error tracking and recovery during boot process
- **Progress Tracking**: Real-time boot progress reporting

### Logger (`logger.h/.cpp`)
**Purpose**: Centralized logging system for the entire application.

**Key Features**:
- **Multiple Levels**: ERROR, WARN, INFO, DEBUG, VERBOSE
- **ESP_LOG Integration**: Compatible with ESP-IDF logging macros
- **Thread-Safe**: Safe for use across FreeRTOS tasks

### Task Scheduler (`task_scheduler.h/.cpp`)
**Purpose**: Centralized task management system for coordinating all FreeRTOS tasks in the ESP32 voice assistant.

**Key Features**:
- **Task Lifecycle**: Initialize, start, and stop all application tasks
- **Priority Management**: Configurable task priorities and stack sizes
- **Resource Coordination**: Prevent task conflicts and ensure proper initialization order
- **Error Handling**: Task creation failure detection and reporting
- **Status Monitoring**: Track running tasks and their states

**Core Tasks Managed**:
- **Main Application Task**: Core voice assistant logic, WiFi handling, audio processing
- **Future Tasks**: Speech recognition, UI updates, network services

**API Methods**:
```cpp
// Lifecycle
static void init();             // Initialize task scheduler
static void startTasks();       // Create and start all tasks
static void stopTasks();        // Stop and clean up tasks

// Task Management
static bool isRunning();        // Check if tasks are active
static uint32_t getTaskCount(); // Get number of running tasks

// Internal Methods
static void mainTask(void* parameter);  // Main application task function
```

**Task Configuration**:
```cpp
// Main task settings
#define MAIN_TASK_STACK_SIZE 4096
#define MAIN_TASK_PRIORITY 1
#define MAIN_TASK_NAME "MainAppTask"
```

**Usage Example**:
```cpp
#include "tasks/task_scheduler.h"

// Initialize in setup()
TaskScheduler::init();

// Start all tasks
TaskScheduler::startTasks();

// Tasks run automatically
// Main loop becomes idle (vTaskDelete)
```

## 🔄 Task Architecture

### Task Hierarchy
```
setup() → TaskScheduler::init()
    ↓
TaskScheduler::startTasks() → xTaskCreate(mainTask, ...)
    ↓
mainTask() → WiFi handling, audio processing, UI updates
```

### Task Communication
- **Direct Calls**: Services called directly from tasks
- **Shared Resources**: Protected access to global instances
- **Event-Driven**: Tasks respond to events and sensor data

### Memory Management
- **Stack Allocation**: Each task has dedicated stack space
- **Heap Usage**: Minimal heap allocation in tasks
- **Static Instances**: Global service instances shared across tasks

## ⚙️ Configuration

Task settings in code:
```cpp
// Task parameters
BaseType_t result = xTaskCreate(
    mainTask,           // Task function
    "MainAppTask",      // Task name
    4096,              // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority (1-5, higher = more priority)
    &mainTaskHandle    // Task handle
);
```

## 🔧 Adding New Tasks

### 1. Define Task Function
```cpp
void TaskScheduler::myNewTask(void* parameter) {
    Logger::info("TASK", "My new task started");

    while (true) {
        // Task logic here
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 second delay
    }
}
```

### 2. Add to Task Creation
```cpp
void TaskScheduler::startTasks() {
    // ... existing tasks

    // Add new task
    xTaskCreate(
        myNewTask,
        "MyNewTask",
        2048,
        NULL,
        1,
        NULL
    );
}
```

### 3. Update Task Cleanup
```cpp
void TaskScheduler::stopTasks() {
    // ... existing cleanup

    // Add new task cleanup if needed
}
```

## 📊 Task Monitoring

### Performance Metrics
- **CPU Usage**: Monitor task execution time
- **Stack Usage**: Check for stack overflows
- **Memory**: Track heap allocation per task

### Debug Information
```cpp
// Get task information
TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();
TaskStatus_t taskStatus;
vTaskGetInfo(taskHandle, &taskStatus, pdTRUE, eInvalid);

// Log task details
Logger::info("TASK", "Task %s - Stack high water: %d",
             taskStatus.pcTaskName,
             taskStatus.usStackHighWaterMark);
```

## 🐛 Troubleshooting

### Common Issues
- **Stack Overflow**: Increase stack size or reduce local variables
- **Priority Conflicts**: Adjust task priorities to prevent starvation
- **Resource Contention**: Use mutexes for shared resource access
- **Timing Issues**: Ensure proper delays between task iterations

### Task States
- **Running**: Currently executing
- **Ready**: Waiting for CPU time
- **Blocked**: Waiting for event/delay
- **Suspended**: Explicitly suspended
- **Deleted**: Terminated

## 📈 Performance Optimization

### Best Practices
- **Minimal Stack**: Use smallest possible stack size
- **Short Execution**: Keep task iterations brief
- **Proper Delays**: Use `vTaskDelay()` instead of busy waiting
- **Priority Levels**: Critical tasks get higher priority

### Memory Considerations
- **Static Allocation**: Prefer static over dynamic memory
- **Stack vs Heap**: Use stack for temporary data
- **Task Local**: Minimize global variable usage

## 🔗 Dependencies

- **FreeRTOS**: ESP32 RTOS kernel
- **ESP-IDF**: Hardware abstraction layer
- **Logger**: Task event logging
- **All Services**: WiFi, audio, network services

## 🎯 Task Design Patterns

### Event-Driven Tasks
```cpp
void eventDrivenTask(void* parameter) {
    while (true) {
        // Wait for event
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Process event
        handleEvent();
    }
}
```

### Periodic Tasks
```cpp
void periodicTask(void* parameter) {
    const TickType_t period = pdMS_TO_TICKS(1000);

    while (true) {
        // Do periodic work
        doPeriodicWork();

        // Wait for next period
        vTaskDelay(period);
    }
}
```

### Sensor Monitoring Tasks
```cpp
void sensorTask(void* parameter) {
    while (true) {
        // Read sensor
        int value = readSensor();

        // Process if changed
        if (value != lastValue) {
            processSensorData(value);
            lastValue = value;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### Touch Sensor (`touch_sensor.h/.cpp`)
**Purpose**: Capacitive touch gesture detection and processing for user interaction.

**Key Features**:
- **Gesture Detection**: One tap, double tap, triple tap, hold tap, long tap
- **Debouncing**: Prevents false triggers with configurable debounce delay
- **Multi-Tap Window**: Time-based detection for rapid tap sequences
- **Timeout Protection**: Prevents stuck states with hold timeouts
- **Event Callbacks**: Configurable callbacks for each gesture type

**Supported Gestures**:
- **ONE_TAP**: Quick tap (< 500ms) - triggers single action
- **DOUBLE_TAP**: Two quick taps within 500ms window
- **TRIPLE_TAP**: Three quick taps within 500ms window
- **HOLD_TAP**: Hold 1-3 seconds
- **LONG_TAP**: Hold > 3 seconds

**API Methods**:
```cpp
// Lifecycle
static bool init();                    // Initialize touch sensor
static void deinit();                  // Clean shutdown

// Configuration
static bool setCallback(Gesture gesture, std::function<void()> callback);

// Debug/Testing
static int getTouchValue();            // Raw sensor value
static bool isCurrentlyTouching();     // Current touch state
```

**Usage Example**:
```cpp
#include "infrastructure/touch_sensor.h"

// Initialize in boot
TouchSensor::init();

// Set gesture callbacks
TouchSensor::setCallback(TouchSensor::ONE_TAP, []() {
    voiceCommandHandler.startListening();
});

TouchSensor::setCallback(TouchSensor::DOUBLE_TAP, []() {
    // Toggle silent mode
});
```

**Configuration** (in `config.h`):
```cpp
#define TOUCH_SENSOR_PIN GPIO_NUM_46
#define TOUCH_THRESHOLD 50
#define TOUCH_POLL_INTERVAL_MS 50
#define TOUCH_ONE_TAP_MAX_MS 500
#define TOUCH_DOUBLE_TRIPLE_WINDOW_MS 500
#define TOUCH_HOLD_MIN_MS 1000
#define TOUCH_LONG_MIN_MS 3000
#define TOUCH_DEBOUNCE_MS 100
```

**Gesture Detection Logic**:
- **Polling**: 50ms intervals for responsive detection
- **State Machine**: Tracks touch start/end with debouncing
- **Tap Counting**: Accumulates taps within time window
- **Timeout Handling**: Resets state on prolonged touch or inactivity

**Integration**:
- **Boot Phase**: Initialized in HARDWARE_INIT
- **Callbacks**: Set in APPLICATION_START phase
- **Threading**: Runs in dedicated FreeRTOS task
- **Logging**: Debug/info messages for gesture events