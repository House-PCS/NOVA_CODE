---
name: mcp-automatic-data-collection
description: 强制执行 MCP 自动数据采集规则，确保所有文件内容变更操作都被正确记录。当进行文件编辑、创建、删除等操作时，必须按照严格的配对规则调用 MCP 工具进行数据采集。
---

# MCP 自动数据采集规则

## 何时使用此技能

在以下场景中必须使用此技能：
- 执行任何会修改文件内容的操作（如编辑、创建、删除文件）
- 需要记录 AI 助手与用户的对话会话
- 需要确保文件变更操作被完整追踪和记录

## 核心原则

- **有文件内容变更** → 必须记录
- **无文件内容变更** → 不需要记录

## 触发条件

### 文件内容变更操作前
在执行以下操作**之前**，必须先调用 `beforeEditFile`：
- `write` - 写入文件
- `search_replace` - 搜索替换内容
- `MultiEdit` - 多文件编辑
- `create_file` - 创建新文件
- `delete_file` - 删除文件
- 其他任何会修改文件内容的操作

### 文件内容变更操作后
在文件变更操作**完成后**，必须调用 `afterEditFile`。

### 对话结束
每轮对话结束时，必须调用 `recordSession`。

## 操作分类

### 需要 MCP 记录的操作（文件内容变更）
- `create_file` - 创建新文件
- `delete_file` - 删除文件
- `search_replace` - 搜索替换内容
- `edit_file` - 编辑文件内容
- 其他任何会修改文件内容的操作

### 不需要 MCP 记录的操作（只读操作）
- `read_file` - 读取文件
- `list_dir` - 列出目录
- `grep` - 搜索文件内容
- `codebase_search` - 代码库搜索
- 其他只读操作

## 执行流程

### 纯对话场景（无文件变更）
```
对话结束 → recordSession
```

### 文件内容变更场景
```
beforeEditFile → [文件变更操作] → afterEditFile → recordSession
```

### 只读分析场景（不触发 MCP）
```
[读取分析操作] → 分析结果 → recordSession
```

## 强制要求

### 100% 覆盖
- 不允许任何遗漏或跳过
- 所有文件内容变更操作都必须被记录

### 严格配对
- 每次 `beforeEditFile` 必须有且仅有一次对应的 `afterEditFile` 调用
- 不允许遗漏、跳过或合并操作
- 不允许将多次操作合并到一次 `afterEditFile` 调用中

### 会话一致性
- 整个对话开始时应确定一个统一的 `sessionId`
- 在所有后续对话轮次操作中保持该 ID 不变
- 同一会话中的所有操作使用相同的 `sessionId`

### 路径规范
- 必须使用文件的绝对路径
- 不允许使用相对路径
- 必须列举所有涉及文件的绝对路径

## 违规处理

### 即时检测
- 每次文件操作后立即自检配对完整性
- 检查是否所有 `beforeEditFile` 都有对应的 `afterEditFile`
- 检查是否所有文件变更操作都被正确记录

### 强制纠正
- 发现遗漏立即停止当前操作
- 补充缺失的 MCP 工具调用
- 确保操作流程完整后再继续

### 重新执行
- 违规操作必须重新执行整个流程
- 确保符合所有强制要求后再继续

## 常见违规案例

### ❌ 错误示例 1：合并记录
```typescript
// 错误：将多次操作合并到一次 afterEditFile
beforeEditFile(file1)
editFile(file1)
editFile(file2)  // 错误：file2 没有 beforeEditFile
afterEditFile(file1)  // 错误：只记录了 file1
```

### ✅ 正确示例 1：分别记录
```typescript
// 正确：每个文件分别记录
beforeEditFile(file1)
editFile(file1)
afterEditFile(file1)

beforeEditFile(file2)
editFile(file2)
afterEditFile(file2)
```

### ❌ 错误示例 2：遗漏配对
```typescript
// 错误：beforeEditFile 后未调用 afterEditFile
beforeEditFile(file1)
editFile(file1)
// 遗漏了 afterEditFile
```

### ✅ 正确示例 2：完整配对
```typescript
// 正确：完整的配对流程
beforeEditFile(file1)
editFile(file1)
afterEditFile(file1)
```

### ❌ 错误示例 3：跳过记录
```typescript
// 错误：直接进行文件变更操作而未调用 MCP 工具
editFile(file1)  // 错误：没有 beforeEditFile
```

### ✅ 正确示例 3：完整记录
```typescript
// 正确：完整的记录流程
beforeEditFile(file1)
editFile(file1)
afterEditFile(file1)
```

### ❌ 错误示例 4：路径错误
```typescript
// 错误：使用相对路径
beforeEditFile('./src/file.ts')  // 错误：相对路径
```

### ✅ 正确示例 4：绝对路径
```typescript
// 正确：使用绝对路径
beforeEditFile('/absolute/path/to/src/file.ts')
```

### ❌ 错误示例 5：错误触发
```typescript
// 错误：对只读操作也调用 beforeEditFile/afterEditFile
beforeEditFile(file1)  // 错误：read_file 是只读操作
readFile(file1)
afterEditFile(file1)  // 错误：read_file 不应该触发 MCP
```

### ✅ 正确示例 5：只读操作不触发
```typescript
// 正确：只读操作不触发 MCP 工具
readFile(file1)  // 只读操作，不需要 MCP 调用
```

## 验证清单

在执行文件变更操作时，请检查以下项目：

- [ ] 是否在文件变更操作前调用了 `beforeEditFile`？
- [ ] 是否在文件变更操作后调用了 `afterEditFile`？
- [ ] 每个 `beforeEditFile` 是否都有对应的 `afterEditFile`？
- [ ] 是否使用了文件的绝对路径？
- [ ] 是否在整个对话中保持 `sessionId` 一致？
- [ ] 是否在对话结束时调用了 `recordSession`？
- [ ] 是否对只读操作错误地调用了 MCP 工具？
- [ ] 是否将多次操作合并到一次 `afterEditFile` 调用中？

## 最佳实践

1. **提前规划**：在执行文件变更操作前，先规划好需要调用的 MCP 工具
2. **即时记录**：每次文件变更后立即调用对应的 MCP 工具，不要延迟
3. **严格配对**：确保每个 `beforeEditFile` 都有对应的 `afterEditFile`
4. **路径检查**：始终使用绝对路径，避免路径错误
5. **会话管理**：在对话开始时确定 `sessionId`，并在整个对话中保持一致
6. **错误恢复**：如果发现遗漏，立即停止并补充缺失的调用

## 注意事项

- 此规则是**强制执行**的，不允许任何例外
- 违反规则可能导致数据采集不完整，影响后续分析
- 如果对某个操作是否需要记录有疑问，优先选择记录
- 只读操作（如 `read_file`）不需要调用 MCP 工具

