# AGENTS.md

## 项目概览

前端-only 的 admin 脚手架，源码位于 `web/`，运行时使用 Bun。

- **运行时**: Bun（不是 npm/yarn/pnpm）
- **前端**: React 19, Vite（root 在 `./web`）, Ant Design 6, Tailwind CSS 4, TanStack Query, Zustand
- **构建产物**: `dist/web/`

## 开发者命令

```bash
bun run dev          # Vite dev server (5173)
bun run dev:web      # 同 bun run dev
bun run build        # Vite build -> dist/web
bun run build:web    # 同 bun run build
bun run start        # vite preview
bun run lint         # biome lint .
bun run typecheck    # tsc --noEmit
bun run format       # biome format --write .
bun run format:check # biome format check .
```

## 环境配置

- Vite 通过 `envDir` 从项目根目录读取 env
- 前端环境变量使用 `VITE_` 前缀

## 目录组织

```text
web/
├── components/     # 公共组件
├── pages/          # 页面
├── hooks/          # 自定义 Hooks
├── store/          # Zustand 状态
├── providers/      # React Providers
├── utils/          # 工具函数
├── routes/         # 路由配置
├── layouts/        # 布局组件
├── config/         # 应用配置
├── styles/         # 全局样式
└── main.tsx        # 前端入口
```

## 前端模块规范

`web/pages/<module>/` 下按模块组织：

| 文件           | 职责                                        |
| -------------- | ------------------------------------------- |
| `index.tsx`    | 页面组件（CRUD UI）                         |
| `*.types.ts`   | 类型定义 + React Query queryKeys            |
| `*.api.ts`     | API 请求（axios 封装）                      |
| `*.service.ts` | React Query Hooks（useQuery / useMutation） |
| `*.schema.ts`  | Zod 验证 schema（表单验证）                 |

## 代码风格

- **Prettier/Biome**: 4 空格缩进，单引号
- **TypeScript**: 严格模式，`moduleResolution: bundler`
- **CSS**: Tailwind CSS 4（通过 `@tailwindcss/vite`），Ant Design cssinjs
- **路径别名**: `@/` 映射 `./web/*`

## 注意事项

- 不要使用 npm/yarn/pnpm，统一使用 Bun
- Vite root 是 `./web`
- 本分支不包含后端实现；前端 API 仍通过 `web/pages/**/*.api.ts` 和 `web/utils/http.ts` 维护
