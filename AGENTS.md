# AGENTS.md

## 项目概述

Monorepo，前后端分离但统一维护在同仓库：

| 模块       | 技术栈                                                         | 入口                |
| ---------- | -------------------------------------------------------------- | ------------------- |
| `web/`     | React 19 + Vite + React Router + TanStack Query + Ant Design 6 | `web/main.tsx`      |
| `service/` | Bun + Hono + TypeORM + MySQL                                   | `service/server.ts` |

- 根目录唯一 `package.json`（**不要创建 `web/package.json` 或 `service/package.json`**）
- TypeScript 配置统一在根目录（`tsconfig.json`）
- Vite 构建输出到 `dist/web`，后端构建产物为 `dist/server.js`

## 开发命令

```bash
bun run dev         # 同时启动后端(1102)和前端(5173)
bun run dev:web     # 仅前端
bun run dev:service # 仅后端
bun run build       # 构建两者
bun run lint        # 类型检查 (tsc --noEmit)
bun run format      # 格式化所有文件
bun run format:check  # 检查格式化状态
```

**首次 setup**：

1. `bun install`
2. `cp .env.example .env` — 修改数据库和 JWT 配置
3. `bun run dev`

**端口约定**：后端默认 `1102`，Vite 开发服务器代理 `/api` 到 `127.0.0.1:1102`。

**不要提交**：`dist/`、`web/dist/`、`tsconfig.web.tsbuildinfo`

## 路径别名

`@/*` 在 tsconfig 中映射为 `["web/*", "service/*"]`。前端代码中 `@/` 等价于 `web/` 根路径。

## API 与数据类型约定

### 请求/响应

- **API payload**：snake_case
- **分页结构**：`{ list: T[], total: number, page: number, pageSize: number, totalPages?: number }`
- **响应封装**：`{ code: 0|number, message: string, data?: T }`
- **错误码**：非 0 为错误

### TypeScript 命名

| 类型                      | 用途        | 示例            |
| ------------------------- | ----------- | --------------- |
| `Item`                    | 列表项/详情 | `UserItem`      |
| `Query`                   | 查询参数    | `UserQuery`     |
| `CreateDto` / `UpdateDto` | 写入参数    | `CreateUserDto` |
| `Option`                  | 选择器选项  | `UserOption`    |
| `TreeItem`                | 树形数据    | -               |

### 数据库字段命名

- snake_case（与 API 一致）
- 不额外添加 camelCase 别名，除非有明确迁移方案

## 前端架构

### 目录结构

```
web/
  components/     # 共享 UI 组件
  config/         # App 配置（app.ts, routes 等）
  hooks/          # 可复用 hooks（useMutation, useDynamicRoutes, useInitAuth 等）
  layouts/        # AdminLayout 等页面外壳
  pages/          # 路由页面（采用页面级组织，按模块分组）
    home/         # 首页
    system/       # 系统管理模块（user, role, department, menu）
  providers/      # TanstackQuery、Message 等顶层 provider
  routes/         # 路由配置与守卫
  store/          # Zustand 状态管理（authStore, tabsStore）
  styles/         # 全局样式
  utils/          # http（axios封装）、tree、query、icon 等工具
  types/          # 前端共享类型（PaginatedResult 等）
  public/         # 静态资源
```

### 页面组织模式（重要）

每个功能模块（如 user）采用就近组织，**不在 `web/services/` 集中管理**：

```
pages/system/user/
  index.tsx       # 页面主入口
  user.types.ts   # 类型定义 + QueryKeys
  user.api.ts     # API 调用函数
  user.service.ts # TanStack Query hooks
  user.schema.ts  # Zod 验证 schema
```

### 关键模式

**路由**：Hash 路由（`createHashRouter`），动态菜单驱动，通过 `pages/index.ts` 的 `registerPage` 注册。

**数据获取**：使用 `useMutationWithMessage`（自动 toast）和 `useSaveMutation`（create/update 合一）封装 mutation。

**HTTP**：基于 axios，响应拦截器自动处理 token 刷新和错误提示。

## 后端架构

### 目录结构

```
service/
  config/         # data.ts（DataSource + RepositoryManager）
  core/           # env.ts（dotenv 加载）、hono.env.ts（AppEnv 类型）
  modules/        # 功能模块（common/, data/, system/）
    common/       # http.ts（响应封装 R）、types.ts（BaseEntity、分页工具）
    system/       # auth/, user/, role/, menu/, department/
    data/         # device/, variable/ 等数据模块
  utils/          # jwt、bcrypt、logger、tree 工具
  server.ts       # 启动入口（Hono 实例 + 路由注册）
```

### 后端模块结构（每个功能模块）

```
modules/system/user/
  user.entity.ts   # TypeORM 实体（继承 BaseEntity）
  user.types.ts    # DTO、Status 类型
  user.schema.ts  # Zod 验证 schema
  user.service.ts # 业务逻辑
  user.route.ts   # Hono 路由（权限中间件）
  user.error.ts   # 业务错误定义
```

### 关键模式

- **权限控制**：通过 `authMiddleware` + `requirePermission('system:user:query')` 中间件
- **响应格式**：使用 `R.ok(c, data)` / `R.page(c, data)` / `R.created(c)` 等
- **分页**：通过 `normalizePagination` + `paginate(qb, pagination, transform)` 工具
- **种子数据**：`userService.seedAdmin()` 在启动时自动创建管理员账户

## 强耦合区域（谨慎修改）

- **菜单/认证/角色/用户/部门模块**：相互依赖，修改需检查联动影响
- **WebSocket 事件名**与 React Query key 必须保持一致
- **TypeORM 实体变更**需同步检查依赖的 route 和 service

## 验证流程

1. `bun run lint` — 类型检查必须通过
2. `bun run build` — 如果改动影响前端或后端构建

## 代码格式

| 规则     | 配置                 |
| -------- | -------------------- |
| 单行长度 | 100                  |
| 缩进     | 4 空格               |
| 引号     | 单引号（JSX 双引号） |
| 分号     | 必须                 |
| 尾逗号   | ES5（多行）          |
| 箭头函数 | 始终括号             |

**Prettier 规则**见 `.prettierrc`，Tailwind class 排序自动处理。

## 项目约定

- 优先小而局部的改动，不轻易做结构性重写
- 不要移动源码文件到 `web/` 或 `service/` 之外
- 删除无调用方的死代码，而非继续增加兼容层
- 修改共享点（query key、路由守卫、菜单权限、实体字段）前先检查联动影响

详细架构文档见 [`README.md`](./README.md)。
