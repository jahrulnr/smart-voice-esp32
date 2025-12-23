#pragma once

#include <Arduino.h>
#include <map>
#include <vector>
#include <functional>

namespace TaskManager {
	using TaskFunction = std::function<void(void)>;
	using LoopTaskFunction = std::function<void(void*)>;
	
	enum class TaskStatus {
		WAITING,
		INPROGRESS,
		PAUSED,
		DONE,
		FAILED,
		EXTERNAL_TASK  // For tasks not created by TaskManager
	};
	
	struct TaskConfig {
		String name;
		uint32_t stackSize = 8192;
		UBaseType_t priority = 1;
		BaseType_t coreId = tskNO_AFFINITY;
		String description = "";
		bool isLoop = false;
		void* params = nullptr;
	};
	
	struct TaskInfo {
		String taskId;
		String name;
		TaskStatus status;
		unsigned long createdAt;
		unsigned long startedAt;
		unsigned long completedAt;
		String description;
		TaskHandle_t handle;
		BaseType_t coreId;
		UBaseType_t priority;
		bool isLoop;
		bool isExternal = false;  // Flag for external tasks
		uint32_t stackSize = 0;    // Stack size allocated
		uint32_t stackFreeMin = 0; // Minimum free stack (high water mark)
		uint32_t stackUsed = 0;    // Current stack usage
	};
	
	// Core task management functions
	String createTask(TaskFunction function, const TaskConfig& config);
	String createLoopTask(LoopTaskFunction function, const TaskConfig& config);
	String createTaskOnCore(TaskFunction function, const String& name, uint32_t stackSize = 8192, 
	                       UBaseType_t priority = 1, BaseType_t coreId = 1, const String& description = "");
	String createLoopTaskOnCore(LoopTaskFunction function, const String& name, uint32_t stackSize = 8192,
	                           UBaseType_t priority = 1, BaseType_t coreId = 1, const String& description = "", void* params = nullptr);
	
	// Task information and control
	TaskStatus getTaskStatus(const String& taskId);
	TaskInfo getTaskInfo(const String& taskId);
	std::vector<TaskInfo> getAllTasks();
	std::vector<TaskInfo> getTasksByStatus(TaskStatus status);
	std::vector<TaskInfo> getTasksByCore(BaseType_t coreId);
	bool stopTask(const String& taskId, bool removeFromRegistry = true);
	bool pauseTask(const String& taskId);
	bool resumeTask(const String& taskId);
	void cleanupCompletedTasks();
	bool removeTask(const String& taskId);
	int getTaskCount();
	int getTaskCountByStatus(TaskStatus status);
	
	// External task scanning
	void scanExternalTasks();
	std::vector<TaskInfo> getExternalTasks();
	bool isTaskExternal(const String& taskId);
	void updateTaskMemoryUsage(const String& taskId);
	void updateAllTasksMemoryUsage();
	bool deleteExternalTask(const String& taskId);  // Safely delete external tasks
}

// Backward compatibility namespace - moved to global level
namespace Command {
	using cmd = TaskManager::TaskFunction;
	using TaskStatus = TaskManager::TaskStatus;
	using TaskInfo = TaskManager::TaskInfo;
	
	inline String Send(cmd command, int priority = 1, const String& description = "", uint32_t stackSize = 8192) {
		TaskManager::TaskConfig config;
		config.name = "CommandTask";
		config.stackSize = stackSize;
		config.priority = priority;
		config.coreId = 1;
		config.description = description;
		return TaskManager::createTask(command, config);
	}
	
	inline TaskStatus GetTaskStatus(const String& taskId) { return TaskManager::getTaskStatus(taskId); }
	inline TaskInfo GetTaskInfo(const String& taskId) { return TaskManager::getTaskInfo(taskId); }
	inline std::vector<TaskInfo> GetAllTasks() { return TaskManager::getAllTasks(); }
	inline std::vector<TaskInfo> GetTasksByStatus(TaskStatus status) { return TaskManager::getTasksByStatus(status); }
	inline bool StopTask(const String& taskId, bool removeFromRegistry = true) { return TaskManager::stopTask(taskId, removeFromRegistry); }
	inline bool PauseTask(const String& taskId) { return TaskManager::pauseTask(taskId); }
	inline bool ResumeTask(const String& taskId) { return TaskManager::resumeTask(taskId); }
	inline void CleanupCompletedTasks() { TaskManager::cleanupCompletedTasks(); }
	inline bool RemoveTask(const String& taskId) { return TaskManager::removeTask(taskId); }
	inline int GetTaskCount() { return TaskManager::getTaskCount(); }
	inline int GetTaskCountByStatus(TaskStatus status) { return TaskManager::getTaskCountByStatus(status); }
	inline void ScanExternalTasks() { TaskManager::scanExternalTasks(); }
	inline std::vector<TaskInfo> GetExternalTasks() { return TaskManager::getExternalTasks(); }
	inline bool IsTaskExternal(const String& taskId) { return TaskManager::isTaskExternal(taskId); }
	inline void UpdateTaskMemoryUsage(const String& taskId) { TaskManager::updateTaskMemoryUsage(taskId); }
	inline void UpdateAllTasksMemoryUsage() { TaskManager::updateAllTasksMemoryUsage(); }
	inline bool DeleteExternalTask(const String& taskId) { return TaskManager::deleteExternalTask(taskId); }
}