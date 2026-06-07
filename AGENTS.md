# AGENTS.md

## 项目概览

前端-only 的 admin 脚手架，源码位于 `web/`，运行时使用 Node.js。

- **运行时**: Node.js + npm
- **前端**: React 19, Vite（root 在 `./web`）, Ant Design 6, Tailwind CSS 4, TanStack Query, Zustand
- **构建产物**: `dist/web/`

## 开发者命令

```bash
npm run dev          # Vite dev server (5173)
npm run dev:web      # 同 npm run dev
npm run build        # Vite build -> dist/web
npm run build:web    # 同 npm run build
npm run start        # vite preview
npm run lint         # biome lint .
npm run typecheck    # tsc --noEmit
npm run format       # biome format --write .
npm run format:check # biome format check .
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

- 使用 npm 管理依赖和运行脚本
- Vite root 是 `./web`
- 本分支不包含后端实现；前端 API 仍通过 `web/pages/**/*.api.ts` 和 `web/utils/http.ts` 维护
