# AGENTS.md

## 项目概览

前后端分离的 admin 脚手架。前端基于 React + Vite，后端用 **C++23 + Ruvia** 重写并通过 **vcpkg + CMake** 构建，目录组织与 main 分支保持一致。

- **前端运行时**: Node.js + npm；React 19 / Vite / Ant Design 6 / Tailwind 4 / TanStack Query / Zustand
- **后端运行时**: C++23 / Ruvia v0.0.4 / asio / mimalloc / MariaDB / OpenSSL / ZLIB
- **构建产物**: `build/web/`（前端）；`build/server`（后端可执行文件）

## 开发者命令

### 前端

```bash
npm run dev          # Vite dev server (5173)
npm run build        # Vite build -> build/web
npm run lint         # biome lint .
npm run typecheck    # tsc --noEmit
```

### 后端（vcpkg + CMake）

```bash
# 配置 + 构建（vcpkg 自带 manifest 模式自动解析依赖）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release

# 运行
./build/server                  # 默认监听 0.0.0.0:1102
```

`.vscode/` 已被 Git 忽略。VSCode/CMake Tools 构建时可在本地工作区 settings 中配置
`CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake` 与
`VCPKG_TARGET_TRIPLET=x64-windows-static`。命令行构建时请先设置 `VCPKG_ROOT`，
或手动传入等价的 `CMAKE_TOOLCHAIN_FILE`。

## 环境配置

复制 `.env.example` -> 项目根目录 `.env`；CMake 配置时会自动同步到可执行文件同目录的 `build/.env`，后端启动只读取该文件：

| 变量                    | 说明                                  |
| ----------------------- | ------------------------------------- |
| `HOST` / `PORT`         | 监听地址；默认 `0.0.0.0:1102`         |
| `WORKER_THREADS`        | Ruvia 工作线程数（默认 2）            |
| `DB_HOST` / `DB_PORT`   | MariaDB 连接参数                      |
| `DB_USERNAME` / `DB_PASSWORD` / `DB_DATABASE` | DB 凭证                |
| `JWT_SECRET`            | 必填，签发 access token 的密钥        |
| `JWT_REFRESH_SECRET`    | 可选，refresh token 密钥（缺省同 JWT_SECRET） |
| `JWT_EXPIRES_IN`        | access token 时效（如 `1d`, `2h`）    |
| `JWT_REFRESH_EXPIRES_IN`| refresh token 时效                    |

服务启动时会通过 Ruvia `DbMigrator` 执行编译进二进制的 schema 迁移；第一次 `GET /api/health` 会触发懒加载 `seedAdmin()`：自动创建 `superadmin` 角色与 `admin/123456` 账户。

## 目录组织

```text
web/                              # 前端源码（Vite root）
├── components/                   # 通用组件（Breadcrumb / FormModal / PageTabs 等）
├── config/                       # 前端应用配置
├── hooks/                        # React hooks
├── layouts/                      # 布局组件
├── pages/                        # 页面模块
│   ├── home/                     # 首页
│   ├── login/                    # 登录模块（api / schema / service / types / page）
│   └── system/                   # 系统管理
│       ├── user/                 # 用户管理
│       ├── role/                 # 角色管理
│       ├── menu/                 # 菜单管理
│       └── dept/                 # 部门管理
├── providers/                    # 全局 Provider
├── public/                       # 静态资源
├── routes/                       # 路由注册
├── store/                        # Zustand store
├── styles/                       # 全局样式 / Tailwind 入口
├── utils/                        # 前端工具
├── index.html                    # Vite HTML 入口
└── main.tsx                      # React 入口

service/                          # 后端源码（C++ header-only 风格）
├── common/                       # 通用响应 / AppError / 分页归一化 / DTO 响应壳
├── config/                       # schema.h（编译期 schema 迁移）
├── middleware/                   # auth / logger / permission 中间件
├── modules/                      # 业务模块
│   └── system/                   # 系统模块
│       ├── auth/                 # 认证
│       ├── user/                 # 用户管理
│       ├── role/                 # 角色管理
│       ├── menu/                 # 菜单管理
│       └── dept/                 # 部门管理
├── utils/                        # jwt / password 工具
└── server.cpp                    # 入口：注册控制器 + 启动 ruvia::app()

AGENTS.md                         # Codex / 项目约定文档
CMakeLists.txt                    # 后端单 TU 构建 + 前端 build target + Ruvia FetchContent
vcpkg.json                        # C++ 依赖清单（asio + mimalloc + libmariadb + openssl + zlib）
package.json / package-lock.json  # 前端依赖和脚本
vite.config.ts                    # Vite 配置（web root、/api dev proxy、build 输出）
tsconfig.json / vite-env.d.ts     # TypeScript 配置和声明
biome.json                        # 前端格式化 / lint 配置
.env.example                      # 环境变量示例
.gitattributes                    # Git LF 规则
.clang-format                     # C++ 格式化规则
.gitignore                        # 忽略 build、node_modules、.vscode、.env 等本地文件
```

## 后端模块文件规范

每个业务模块固定文件结构（与 main 分支 TS 模块一一对应）：

| 文件           | 职责                                                      |
| -------------- | --------------------------------------------------------- |
| `*.service.h`  | 业务逻辑：通过 `c.db()` 执行 SQL，返回 Ruvia DTO          |
| `*.controller.h` | `ruvia::Controller<T>` 派生，宏注册路由                |
| `*.schema.h`   | Ruvia 请求校验中间件                                     |
| `*.types.h`    | Ruvia DTO（请求体 / 响应体）；查询参数由 controller 直接读取 |
| `*.error.h`    | `AppErrorDef` 常量集合                                    |

## 架构要点

- **Header-only**：所有业务/工具代码以 `.h` 内联实现，`server.cpp` 是唯一翻译单元，简化构建。
- **Controller 注册**：`RUVIA_CONTROLLER_GROUP("/api/users")` + `RUVIA_ROUTES_BEGIN/END` 静态注册；包含 header 即触发。
- **请求与响应**：
    - JSON 请求体用 `RUVIA_MODEL` + `RUVIA_VALIDATE_JSON`，controller 中通过 `c.valid<T>()` 读取
    - 查询参数由 controller 通过 `c.query("name").toStringView()` / `toInt64()` 读取并传给 service
    - 路径参数由 controller 通过 `c.param("id").toInt64()` 读取并校验
    - 响应使用 `RUVIA_MODEL` DTO，统一 `{ code, message, data }` 壳
- **认证**：受保护 controller 挂 `AuthMiddleware`，JWT 通过 Ruvia `ruvia/auth/Jwt.h` 签发与校验，payload 写入 `c.valid<JwtPayload>(ruvia::Form)`；
  权限校验 `co_await requirePermission(c, "system:user:query")`。
- **权限缓存**：`PermissionService`（`middleware/permission.h`）按用户缓存 60s，
  增删改菜单/角色后调 `clearAllCache()`。
- **错误处理**：业务通过 `throwAppError()` 直接抛 `ruvia::HttpError`，全局 `setErrorHandler` 输出
  `{code, message}` JSON 壳，状态码沿用 `AppErrorDef.status`。
- **DB**：启动时 `DbMigrator` 执行 schema 迁移；Ruvia `useDb(DbConfig)` 注入；查询用
  `c.db().query(sql, params)`，写入/DDL 用 `c.db().execute(sql, params)`；服务层为 raw SQL（Ruvia 当前未带 ORM）。
- **静态资源**：若存在 `build/web/` 则通过 Ruvia `setDocumentRoot()` 提供静态文件；SPA fallback 在全局 404 handler 中只处理非 API、非文件型路径。
- **路径别名**：`#include "service/..."`（CMake target_include_directories 把仓库根加入 include path）。

## 命名约定

- 数据库表：`sys_<name>`（sys_user / sys_role / sys_menu / sys_user_role / sys_role_menu …）
- API 路径：复数名词（`/api/users`、`/api/roles`、`/api/menus`、`/api/depts`、`/api/auth`）
- 权限码：`system:<resource>:<action>`，例如 `system:user:add`
- 响应字段：`{ code, message, data }`；分页用 `data.list / total / page / pageSize / totalPages`

## 常见问题

- **找不到 Ruvia**：CMake 通过 FetchContent 拉取 `hyird/Ruvia`，确认网络或本地缓存可用。
- **JWT_SECRET 未设置**：后端启动后任何登录请求会 500，请在 `.env` 中设置。
- **OpenSSL 未找到**：Ruvia 的 TLS/JWT 和当前密码 PBKDF2 都依赖 OpenSSL，确认 vcpkg 依赖已安装。
- **建表**：无需手动执行 SQL；启动时会执行 `service/config/schema.h` 中编译进二进制的 Ruvia migrations，并记录到项目显式配置的迁移表 `cyra_schema_migrations`（自增 `id` 主键 + 唯一 `migration_id`，用于兼容已有数据库）；迁移 ID 使用四位补零格式（`0001`、`0002` ...）。
