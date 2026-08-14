# AGENTS.md

## 项目概览

前后端分离的 admin 脚手架。前端基于 React + Vite，后端使用 C++23 + Ruvia，
通过 vcpkg + CMake 构建。

- **前端工具链**：Node.js 24 + Bun；React 19 / Vite 8 / Ant Design 6 /
  Tailwind 4 / TanStack Query / Zustand
- **后端工具链**：C++23 / Ruvia `main@ee067028`（core/http/web 三目标）/
  asio / MariaDB / OpenSSL / ZLIB / Brotli / Zstd
- **依赖锁文件**：前端使用 `bun.lock`；后端使用 `vcpkg.json`
- **构建产物**：前端为单文件 `build/web/index.html`；后端为 `build/server`

本文档描述稳定的分层边界和新增代码规则，不维护业务模块的完整清单。实际已有模块、
路由和迁移以仓库源码为准；后续新增模块必须遵守本文中的占位符模板和职责约束。

## 开发者命令

### 前端

```bash
bun install --frozen-lockfile
bun run dev            # Vite dev server，默认 5173
bun run build          # 单文件构建 -> build/web/index.html
bun run lint           # biome lint .
bun run typecheck      # tsc --noEmit
bun run format         # 写入前端格式化结果
bun run format:check   # 只检查前端格式
```

CMake 的集成前端 target 当前通过 `npm run build` 调用同一个 package script，因此执行
完整 CMake 构建的环境仍需提供 Node.js 自带的 npm；依赖安装和锁文件校验以 Bun 为准。

### 后端（vcpkg + CMake）

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release

./build/server          # 默认监听 0.0.0.0:1102
```

`.vscode/` 已被 Git 忽略。VSCode/CMake Tools 可以在本地 workspace settings 中配置
`CMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake` 与
`VCPKG_TARGET_TRIPLET=x64-windows-static`。命令行构建时先设置 `VCPKG_ROOT`，或显式传入
等价的 `CMAKE_TOOLCHAIN_FILE`。

### C++ 格式化

仓库根目录的 `.clang-format` 是唯一 C++ 格式来源。提交前格式化并检查所有项目自有的
`.h` / `.cpp` 文件，不格式化 FetchContent、vcpkg 或 `build/` 下的第三方源码。

```powershell
$cppFiles = rg --files service -g '*.h' -g '*.cpp'
clang-format -i $cppFiles
clang-format --dry-run --Werror $cppFiles
```

## Ruvia 依赖更新约定

Ruvia 上游当前不发布 tag。本项目在 `CMakeLists.txt` 的 `FetchContent_Declare` 中将
`GIT_TAG` 固定为上游 `main` 的完整 commit SHA，以保证构建可复现；文档概览使用同一
commit 的短 SHA。更新时必须：

1. 查询 `hyird/Ruvia` 上游 `main` 的最新 commit，并把完整 SHA 写入 `GIT_TAG`；
2. 一次性把项目代码迁移到该 commit 的公开 API，项目内只保留一套调用方式；
3. 不新增过渡宏、别名、包装器或兼容头；
4. 更新本节和项目概览中的 commit，格式化全部项目 C++ 代码；
5. 至少完成前端 lint/typecheck/build/format 检查和后端 Release 构建。

当前 Ruvia API 基线：

- 请求模型使用 `RUVIA_REQUEST_MODEL(...)`，响应模型使用 `RUVIA_RESPONSE_MODEL(...)`；
  两种角色不得互相嵌套或混用于解析、序列化；
- `fromJson()` / `fromForm()` 只接受请求模型，`toJson()` / `Context::json()` 只接受响应
  模型；运行时形状不得绕过模型边界拼装动态 JSON；
- 字段使用 `RUVIA_REQUIRED_FIELD(...)` / `RUVIA_OPTIONAL_FIELD(...)` 声明，通过
  `get<"field">()` / `set<"field">()` / `ensure<"field">()` / `reset<"field">()` 访问；
- 校验后的 JSON 通过 `c.req().validated<T>()` 读取；
- 普通运行时集合使用 `ruvia::Array<T>`；只有 DTO 直接或间接递归包含自身，或确实需要
  独立对象存储语义时才使用 `ruvia::BoxedArray<T>`，不得仅因元素是 DTO 就改用 boxed 容器；
- 请求级数据使用 `c.bindRequestState(value)` 绑定，并通过 `c.requestState<T>()` 读取；
- MariaDB 配置使用 `ruvia::DbConfig::mariaDb()` 显式选择驱动；`query()` 结果直接作为
  `ruvia::DbRows` 容器使用，字段通过 `value()` 或 `as<T>()` 读取；
- HTTP 状态使用 `ruvia::HttpStatusCode` 和 `ruvia::http_status::*`，不把裸整数直接当作
  框架状态类型；
- 校验错误通过 `HttpErrorInfo::validationIssues()` 暴露的强类型 `ValidationIssue` 集合读取，
  不依赖预序列化的错误详情 JSON；
- 服务监听通过 `App::setListeners()` / `setWorkersPerListener()` 配置，错误与访问日志使用
  `onError()` / `onAccess()` 回调。

业务辅助函数可以继续存在，但框架升级必须修改调用点，不得扩展第二套过渡 API。

## 环境配置

复制 `.env.example` 到项目根目录 `.env`。CMake 会把它同步到后端可执行文件目录，
`app.loadDotenv()` 从可执行文件同目录读取 `.env`。

| 变量 | 说明 |
| --- | --- |
| `HOST` / `PORT` | 监听地址；默认 `0.0.0.0:1102` |
| `WORKER_THREADS` | 每个 Ruvia listener 的 worker 数；默认 2 |
| `DB_HOST` / `DB_PORT` | MariaDB 地址；默认 `127.0.0.1:3306` |
| `DB_USERNAME` / `DB_PASSWORD` / `DB_DATABASE` | MariaDB 凭证与数据库名 |
| `JWT_SECRET` | 签发和验证 access token 的密钥，服务启动必填 |
| `JWT_REFRESH_SECRET` | refresh token 密钥；缺省复用 `JWT_SECRET` |
| `JWT_EXPIRES_IN` | access token 时效，如 `1d`、`2h` |
| `JWT_REFRESH_EXPIRES_IN` | refresh token 时效 |

JWT 时效只接受正整数秒数，或带 `s`、`m`、`h`、`d` 后缀的正整数。服务启动时严格校验
JWT 密钥和时效配置；配置错误直接终止启动，不静默使用回退值。

服务启动时通过 Ruvia `DbMigrator` 执行编译进二进制的 schema 迁移。默认角色、账户、
菜单和权限由幂等迁移创建；`GET /api/health` 只返回健康状态，不承担初始化副作用。

## 目录与组织原则

目录文档只规定边界，不枚举固定业务名称。新增代码按以下抽象结构放置：

```text
web/
├── components/                        # 跨业务复用的 React 组件
├── config/                            # 前端应用配置
├── hooks/                             # 跨业务复用的 React hooks
├── layouts/                           # 应用级布局
├── pages/<domain>/<module>/           # 前端业务模块；按下文模板新增
├── providers/                         # 全局 Provider 装配
├── routes/                            # 路由注册与页面解析
├── store/                             # 跨页面客户端状态
├── styles/                            # 全局样式与 Tailwind 入口
└── utils/                             # 无业务归属的通用前端工具

service/
├── common/                            # 通用 HTTP、响应、错误与数据库辅助
├── config/                            # 进程配置与 schema 迁移
├── middleware/                        # 认证、权限与日志中间件
├── modules/<domain>/<module>/         # 后端业务模块；按下文模板新增
├── utils/                             # 无状态后端工具
└── server.cpp                         # 唯一翻译单元与进程装配入口
```

- 上述 shared layer 名称按当前仓库列出。新增 shared layer 必须先证明其边界不能归入现有目录，
  并同步更新本节；不得为单个业务模块新增顶层目录。
- `<domain>` 表示稳定的业务域，`<module>` 表示域内单一资源或用例；使用小写名称，多个
  单词使用 `snake_case`。
- 凡是同时包含管理页面和后端 API 的功能，前后端必须使用同一组 `<domain>/<module>`：
  `web/pages/<domain>/<module>/` 对应 `service/modules/<domain>/<module>/`。目录对应关系是
  模块所有权契约，不按当前业务名称写死，也不得在两端为同一功能另取不同别名。
- 纯展示页面可以没有后端镜像目录，纯基础设施 API 也可以没有前端页面；这种单边模块不创建
  空目录占位。一旦页面开始拥有业务 API，就必须补齐同路径的后端模块。
- 页面调用其他模块拥有的 API 时，复用 API 所有者导出的 client 和类型，不在调用方目录复制
  endpoint、DTO 或后端实现。跨模块组合不改变 API 的目录所有权。
- 业务代码优先留在所属模块。只有出现真实的跨模块复用后，才提升到共享层；禁止提前创建
  含义模糊的 `common`、`helpers`、`misc` 或大型 `utils` 汇总入口。
- 新增模块不得因为现有业务模块的名称、数量或目录顺序而修改本规范；只有分层规则发生
  变化时才更新本节。
- 本次文档更新不要求为统一外观而迁移既有模块；修改既有模块时应逐步靠拢同一职责边界，
  不能借机扩大改动范围。
- 文件只在承担实际职责时创建，不生成空模板或仅转发的占位文件。

## 前端新增模块规范

前端页面模块采用严格文件白名单，只允许下列文件；不需要的职责可以省略对应文件，但不得
增加其他文件或子目录。模块拥有业务 API 时，下列 `<domain>/<module>` 必须与后端模块路径
完全一致。

```text
web/pages/<domain>/<module>/
├── <module>.types.ts       # API 类型、状态、查询参数和 DTO
├── <module>.schema.ts      # 表单或请求的运行时校验 schema
├── <module>.api.ts         # HTTP endpoint 与序列化边界
├── <module>.service.ts     # 查询、mutation、缓存失效与页面用例编排
└── index.tsx               # 页面入口和 UI 组合
```

- `*.types.ts` 不依赖 React 组件；跨模块引用只导入确实共享的最小类型。
- `*.schema.ts` 只描述校验和类型推导，不发请求、不操作缓存。
- `*.api.ts` 只绑定 HTTP 方法、路径和请求/响应契约，不包含 UI 提示与页面状态。
- `*.service.ts` 负责 TanStack Query、mutation 和缓存策略，不声明 JSX。
- `index.tsx` 负责展示与交互组合，不直接拼接 API URL，也不重新实现 service 中的缓存逻辑。
- `web/pages/<domain>/<module>/` 除模板列出的 `index.tsx`、`<module>.types.ts`、
  `<module>.schema.ts`、`<module>.api.ts`、`<module>.service.ts` 外，不允许出现任何其他文件
  或子目录。
- 页面私有 UI 保留在 `index.tsx`；跨页面复用的 React 组件必须放在 `web/components/`，
  不得在页面模块内创建组件文件或 `components/` 目录。
- 通用分页、查询串、HTTP client、认证状态等基础设施必须复用共享层，不能在业务模块内复制。

## 后端新增模块规范

后端保持 header-only 业务代码，`service/server.cpp` 是唯一翻译单元。后端模块采用严格文件
白名单，只允许下列文件；不需要的职责可以省略对应文件，但不得增加其他文件或子目录：

```text
service/modules/<domain>/<module>/
├── <module>.types.h        # Ruvia 请求/响应 DTO
├── <module>.schema.h       # RUVIA_VALIDATE_JSON 校验器
├── <module>.error.h        # AppErrorDef 领域错误
├── <module>.service.h      # 业务用例、SQL、事务与 DTO 组装
└── <module>.controller.h   # 路由注册和 HTTP 参数边界
```

- 面向页面的 controller 必须放在与页面相同的 `<domain>/<module>` 下；禁止按 HTTP 层、
  数据表名或临时项目代号另建一套无法与页面对应的目录。
- `*.controller.h` 只做路由、中间件、查询/路径参数解析、请求 DTO 读取和响应封装；业务规则放入
  service。
- `*.schema.h` 只定义请求校验；`*.types.h` 只定义 DTO，不执行 I/O。
- `*.service.h` 是当前持久化与业务边界：通过 `c.db()` 执行 SQL，需要原子性时显式开启事务。
- `*.error.h` 只维护稳定的领域错误码、HTTP 状态和用户消息。
- `service/modules/<domain>/<module>/` 除模板列出的 `<module>.types.h`、
  `<module>.schema.h`、`<module>.error.h`、`<module>.service.h`、
  `<module>.controller.h` 外，不允许出现任何其他文件或子目录。
- 模块复杂度必须在上述职责边界内处理。确需新增文件类型时，必须先修改项目级模块规范，
  不得为单个模块增加例外文件、`common.h`、`helper.h` 或子目录。
- 新 controller 必须由唯一翻译单元包含，确保 `RUVIA_CONTROLLER_GROUP` 静态注册生效。
- include 统一使用从仓库根开始的 `#include "service/..."`；CMake 已把仓库根加入 include path。

## 后端架构要点

- **Controller 注册**：使用 `RUVIA_CONTROLLER_GROUP(...)` 与
  `RUVIA_ROUTES_BEGIN/END` 静态注册路由。
- **请求校验**：请求 DTO 用 `RUVIA_REQUEST_MODEL`，响应 DTO 用 `RUVIA_RESPONSE_MODEL`，
  schema 用 `RUVIA_VALIDATE_JSON`，controller 通过 `c.req().validated<T>()` 获取校验结果。
- **查询与路径参数**：Ruvia accessor 返回 `std::optional<std::string_view>`；整数统一通过项目
  公共严格解析函数转换。
- **响应**：Ruvia DTO 统一输出 `{ code, message, data }`；分页 wire 字段为
  `data.list / total / page / page_size / total_pages`。前端内部可以使用 `pageSize` / `totalPages`，
  但必须在 `*.api.ts` 序列化边界完成转换。
- **认证**：受保护 controller 挂 `AuthMiddleware`。JWT 验证结果通过 request state 传递，
  权限检查使用 `co_await requirePermission(c, "<permission-code>")`。
- **权限缓存**：权限服务按用户缓存 60 秒；会改变权限投影的写操作完成后必须清空相关缓存。
- **错误处理**：业务通过 `throwAppError()` 抛 `ruvia::HttpError`，全局 error callback 输出统一
  JSON 错误壳并保留强类型 HTTP status。
- **数据库**：启动时执行迁移并通过 `app.useDb()` 注入连接；查询使用 `query()` 并直接遍历
  返回的 `DbRows`，写入使用 `execute()`，多步写操作使用 transaction。字段通过 `value()` 或
  `as<T>()` 读取；参数容器必须拥有跨 `co_await` 所需的生命周期。
- **静态站点**：只有可执行文件旁存在 `web/` 目录时才调用 `setDocumentRoot()`。SPA fallback
  只处理非 API、非文件型的 GET/HEAD 请求。

## 前端构建约束

- `vite-plugin-singlefile` 将 JavaScript 和 CSS 内联到 `build/web/index.html`；部署物必须保持为
  一个 HTML 文件，除非需求明确改变交付协议。
- favicon 已以内联 data URL 提供。新增运行时资源必须确认能被单文件构建内联，禁止直接新增
  依赖外部 `public/` 文件的生产路径。
- `/api` 开发代理目标端口来自根目录环境配置，缺省为 1102。
- 不得重新加入手写 hash 重命名、手工 chunk 聚合或仅用于掩盖 bundle 体积的告警阈值。

## 命名约定

- 数据库表：`sys_<resource>`；关联表使用 `sys_<left>_<right>`。
- API 路径：小写复数名词，模块 controller 统一持有自己的根路径。
- C++ DTO / class：`PascalCase`；函数和局部变量：`camelCase`；文件和目录按上述
  `<module>.<responsibility>` 模板命名。
- 前端 API JSON 字段沿用 `snake_case`；TypeScript 内部类型名使用 `PascalCase`。
- 权限码：`<domain>:<resource>:<action>`，全部小写。
- 迁移 ID：四位补零字符串。已发布迁移的 ID、SQL 和顺序不可修改，只能追加新迁移。

## 常见问题

- **找不到 Ruvia**：CMake 通过 FetchContent 拉取 `hyird/Ruvia` 的固定 commit。先确认网络，
  再确认 `CMakeLists.txt` 的完整 SHA 与上游 `main` 目标一致。
- **JWT_SECRET 未设置**：服务启动时会拒绝继续；部署前必须在可执行文件旁的 `.env` 中配置，
  不能依赖开发默认值。JWT 时效格式错误同样会终止启动。
- **OpenSSL 未找到**：Ruvia TLS/JWT 和密码 PBKDF2 都依赖 OpenSSL，确认 vcpkg manifest
  依赖已安装。
- **建表**：无需手动执行 SQL；启动时执行 `service/config/schema.h` 中的 Ruvia migrations，
  记录表为 `cyra_schema_migrations`。已发布迁移只能追加，不能原地修改。
- **构建后出现额外静态文件**：先检查是否绕过 `vite-plugin-singlefile`、直接引用 `public/`
  资源或新增了无法内联的动态加载，再决定是否需要调整实现；不要静默改变单文件交付协议。
