# AGENTS.md

## 项目概览

前后端分离的 admin 脚手架，`web/` 为前端，`service/` 为后端。

- **运行时**: Node.js + npm
- **前端**: React 19, Vite（root 在 `./web`）, Ant Design 6, Tailwind CSS 4, TanStack Query, Zustand
- **后端**: Hono, TypeORM, MariaDB, JWT + bcrypt 认证
- **构建产物**: `dist/web/`（前端）, `dist/server.js`（后端）

## 开发者命令

```bash
npm run dev          # 并发启动 service (1102) 和 web (5173)
npm run dev:web      # 仅前端 (Vite dev server, 5173)
npm run dev:service  # 仅后端 (tsx watch, 1102)
npm run build        # 构建全部：web → dist/web, service → dist/server.js
npm run build:web    # Vite build → dist/web
npm run build:service # tsc 构建 service/server.ts → dist/server.js
npm run start        # NODE_ENV=production node dist/server.js
npm run lint         # biome lint .
npm run typecheck    # tsc --noEmit
npm run format       # biome format --write .
npm run format:check # biome format check .
```

## 环境配置

- 首次运行前复制 `.env.example` → `.env`
- 后端从项目根目录读取 `.env`（通过 `service/core/env.ts`）
- Vite 通过 `envDir` 从项目根目录读取 env
- `DB_SYNCHRONIZE=true` 在开发模式下自动创建/迁移 DB 表（TypeORM）
- 服务启动时总是执行 `repo.initialize()` + `userService.seedAdmin()`

## 目录组织

```
web/                          # 前端（Vite root）
├── components/               # 公共组件（FormModal, PageContainer, StatusTag, Breadcrumb, PageTabs, DynamicIcon, ErrorBoundary）
├── pages/                    # 页面（按模块分组）
│   ├── login/                # 登录页
│   └── system/               # 系统管理模块
│       ├── user/             # 用户管理
│       ├── role/             # 角色管理
│       ├── menu/             # 菜单管理
│       └── department/       # 部门管理
│   ├── home/                 # 首页
│   └── index.ts              # 页面导出汇总
├── hooks/                    # 自定义 Hooks（usePermission, useMutation, useInitAuth, useDynamicRoutes, useDebounceFn）
├── store/                    # Zustand 状态（authStore, tabsStore）
├── providers/                # React Providers（TanstackQuery, Message）
├── utils/                    # 工具函数（http, query, tree, types, animations, icon）
├── routes/                   # 路由配置
├── layouts/                  # 布局组件（AdminLayout）
├── config/                   # 应用配置（app.ts）
├── styles/                   # 全局样式
└── main.tsx                  # 前端入口

service/                      # 后端
├── modules/                  # 业务模块
│   ├── system/               # 系统模块
│   │   ├── auth/             # 认证（auth.route, auth.service, auth.middleware, permission.middleware, permission.service）
│   │   ├── user/             # 用户（user.entity, user.service, user.route, user.schema, user.types, user.error）
│   │   ├── role/             # 角色
│   │   ├── menu/             # 菜单
│   │   └── department/       # 部门
│   ├── common/               # 公共模块（http, request, types）
│   └── data/                 # 数据实体（device, variable, variable-history, variable-realtime）
├── core/                     # 核心配置（env.ts, hono.env.ts）
├── config/                   # 数据源配置（data.ts — AppDataSource, RepositoryManager）
├── utils/                    # 工具函数（logger, jwt, bcrypt, tree）
└── server.ts                 # 后端入口

dist/
├── web/                      # 前端构建产物
└── server.js                 # 后端构建产物
```

## 模块文件规范

每个业务模块遵循固定的文件结构：

### 后端（service/modules/<module>/）

| 文件           | 职责                                |
| -------------- | ----------------------------------- |
| `*.entity.ts`  | TypeORM 实体，定义数据库表结构      |
| `*.service.ts` | 业务逻辑，数据库操作                |
| `*.route.ts`   | Hono 路由处理器，参数校验，权限校验 |
| `*.schema.ts`  | Zod 验证 schema                     |
| `*.types.ts`   | TypeScript 类型定义、DTO 接口       |
| `*.error.ts`   | 业务错误码定义                      |

### 前端（web/pages/<module>/）

| 文件           | 职责                                        |
| -------------- | ------------------------------------------- |
| `index.tsx`    | 页面组件（CRUD UI）                         |
| `*.types.ts`   | 类型定义 + React Query queryKeys            |
| `*.api.ts`     | API 请求（axios 封装）                      |
| `*.service.ts` | React Query Hooks（useQuery / useMutation） |
| `*.schema.ts`  | Zod 验证 schema（表单验证）                 |

## 开发规范

### 命名约定

- **数据库表名**: `sys_<name>`（如 `sys_user`, `sys_role`）
- **API 路径**: 复数名词（如 `/api/users`）
- **权限码**: `system:<entity>:<action>` 格式

    ```
    system:user:query   查询
    system:user:add     新增
    system:user:edit    编辑
    system:user:delete  删除
    ```

- **React Query Keys**: 通过 `createQueryKeys` 创建，格式 `['users', 'list', params]`

### API 设计

- GET `/api/<resource>` — 分页列表
- GET `/api/<resource>/options` — 选项列表（选择器用）
- GET `/api/<resource>/:id` — 详情
- POST `/api/<resource>` — 新增
- PUT `/api/<resource>/:id` — 编辑
- DELETE `/api/<resource>/:id` — 删除

响应格式统一使用 `R.page()`（分页）、`R.ok()`（单条）、`R.created()` / `R.updated()` / `R.deleted()`（操作结果）。

### 状态类型命名

```typescript
UserStatus = 'enabled' | 'disabled';
UserQuery; // 查询参数
UserItem; // 列表项/详情
UserOption; // 选择器选项
CreateUserDto; // 新增参数
UpdateUserDto; // 更新参数
```

### 前端类型封装

使用 `namespace` 封装同一前缀的类型，便于模板使用：

```typescript
export namespace User {
    export type Status = UserStatus;
    export type Item = UserItem;
    export type Query = UserQuery;
    export type CreateDto = CreateUserDto;
    export type UpdateDto = UpdateUserDto;
}
```

### 权限控制

- 后端路由使用 `requirePermission('system:user:query')` / `requireAnyPermission([...])` 中间件
- 前端使用 `usePermissions()` Hook 的 `has('system:user:query')` 判断按钮显示

## 架构要点

- **路径别名**: `@/` 仅映射 `./web/*`（tsconfig + vite）；后端使用相对 ESM import
- **Vite root**: `./web` — vite.config.ts, index.html, main.tsx 都在 `web/` 下
- **后端入口**: `service/server.ts` — 注册所有 Hono 路由并服务静态 `dist/web`
- **API 路由**: `/api/auth`, `/api/users`, `/api/roles`, `/api/menus`, `/api/departments`
- **数据库**: MariaDB（非 MySQL），通过 mysql2 驱动；TypeORM 使用实验性装饰器（`emitDecoratorMetadata`）
- **前端路由**: React Router 7（SSR 风格，后端用 `*` 捕获所有请求实现 SPA）
- **数据源管理**: `service/config/data.ts` 提供 `AppDataSource` 和 `repo` 单例
- **密码**: 后端存储 `password_hash`，使用 bcrypt；前端密码字段不能传空字符串

## 代码风格

- **Prettier**: 4 空格缩进，单引号，Tailwind 插件启用
- **TypeScript**: 严格模式，`moduleResolution: bundler`
- **CSS**: Tailwind CSS 4（通过 `@tailwindcss/vite`），Ant Design cssinjs

## 测试

- 未配置测试框架（package.json 中无 test 脚本）
- 通过 `npm run dev` 手动测试

## 常见错误

- 使用其他包管理器而非 npm
- 忘记复制 `.env.example` → `.env`（数据库连接会失败）
- 在没有 `package-lock.json` 的情况下运行 `npm run build`（先执行 `npm install`）
- 在 `web/` 或 `service/` 以外目录编辑文件，期望 Vite 能识别（Vite root 是 `./web`）
