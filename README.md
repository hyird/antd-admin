# antd-admin

一个前后端一体的管理后台脚手架：前端基于 React，后端基于 Bun + Hono + TypeORM。

如果你准备修改本仓库，请先阅读 [`AGENTS.md`](./AGENTS.md)。其中定义了当前架构的开发边界，避免后续改动破坏现有约定。

## 快速开始

1. 安装依赖：`bun install`
2. 准备环境变量：将 `.env.example` 复制为 `.env`，并按本地环境修改数据库与 JWT 配置
3. 启动开发环境：`bun run dev`
4. 默认情况下，后端运行在 `1102` 端口；前端开发服务会通过 `vite` 代理访问后端

`.env.example` 中的关键配置包括：

- `PORT`：后端端口，默认 `1102`
- `DB_HOST` / `DB_PORT` / `DB_USERNAME` / `DB_PASSWORD` / `DB_DATABASE`：MySQL 连接信息
- `JWT_SECRET` / `JWT_REFRESH_SECRET`：认证相关密钥

常用命令：

```bash
bun run dev         # 同时启动前端和后端
bun run dev:web     # 仅启动前端
bun run dev:service # 仅启动后端
bun run build       # 构建前后端
bun run lint        # 执行整个工作区的类型检查
```

## 项目结构

```text
.
├─ web/                   # 前端源码
├─ service/               # 后端源码
├─ package.json           # 根目录唯一 package 清单
├─ tsconfig.json          # 根级 TypeScript 入口配置
├─ tsconfig.web.json      # 前端 TypeScript 配置
├─ tsconfig.service.json  # 后端 TypeScript 配置
├─ vite.config.ts         # 前端构建配置
├─ vite-env.d.ts          # Vite 环境类型声明
└─ AGENTS.md              # 开发约束说明
```

## 核心规则

- `package.json` 和各类 `tsconfig` 文件只保留在仓库根目录。
- 不要重新引入 `web/package.json`、`service/package.json`、`web/tsconfig.json` 或 `service/tsconfig.json`。
- 保持当前 `web/` 和 `service/` 的目录分层不变。
- 优先做小而局部的改动，不要轻易做结构性重写。
- 除非仍有真实调用方依赖，否则不要新增重复的兼容别名或包装类型。

## 前端

- `web/components/`：共享 UI 组件。
- `web/hooks/`：可复用 hooks。
- `web/layouts/`：页面外壳与布局片段。
- `web/pages/`：路由级页面。
- `web/providers/`：顶层 providers。
- `web/routes/`：路由配置与守卫。
- `web/services/`：API hooks、query keys、mutations 和服务辅助逻辑。
- `web/services/common/`：通用服务基础设施。
- `web/store/`：Redux 状态管理。
- `web/types/`：前端类型与 API 数据结构。
- `web/utils/`：工具函数与树结构辅助方法。

## 后端

- `service/modules/`：功能模块。
- `service/core/`：数据库、响应封装与仓储基础设施。
- `service/types/`：后端共享类型。
- `service/utils/`：通用工具函数。
- `service/server.ts`：后端启动入口。

## 数据结构约定

- 面向前端的 API payload 默认使用 `snake_case`。
- TypeScript 命名保持语义清晰：`Item` 表示列表项或详情记录，`Query` 表示查询参数，`CreateDto` / `UpdateDto` 表示写入参数，`TreeItem` 表示树形结构数据。
- 同一字段除非存在明确迁移方案，否则不要再增加一套 camelCase 别名。

## 变更注意事项

- 菜单、认证、角色、用户、部门等模块耦合较紧，修改时要谨慎。
- WebSocket 事件名与 React Query key 需要保持一致。
- 修改 TypeORM 实体时，应同步检查依赖它的路由和服务。
- 下列生成产物不要提交到仓库：`dist/`、`web/dist/`、`tsconfig.web.tsbuildinfo`。

## 协作约定

- 默认保持小而局部的改动，除非明确需要，否则不要做结构性重写。
- 不要把源码文件移出 `web/` 或 `service/`，也不要为了“更整洁”而调整目录结构。
- `web/` 用于 React/Vite 前端，`service/` 用于 Bun/Hono/TypeORM 后端，根目录保留统一的配置文件。
- 尽量沿用现有 API 字段风格与数据结构，不额外增加重复命名或兼容层。
- 如果某个 helper、type 或分支逻辑已经没有真实调用方，优先考虑删除死代码，而不是继续增加中间层。
- 修改共享 query key、路由守卫、菜单权限、实体字段等耦合点时，先检查相关页面与服务的联动影响。

## 验证要求

- 代码改动后运行 `bun run lint`。
- 如果改动可能影响前端或后端构建，运行 `bun run build`。
- 即使改动只影响单侧，也应确认根级别的 lint / build 流程保持通过。

## 安全改动流程

1. 用最小改动解决问题。
2. 保持文件位置稳定，不随意移动目录。
3. 运行 `bun run lint`。
4. 如果改动影响前端、后端或共享基础设施，运行 `bun run build`。
5. 完成前清理生成产物。

## 后续维护建议

- 如果某次重构看起来需要移动目录，先判断是否能用更薄的一层局部修复解决。
- 如果某个字段或类型看起来重复，先确认是否仍被真实调用方使用，再决定是否删除。
- 如果改动涉及共享 query key、请求流或路由守卫，清理前先检查受影响页面。
