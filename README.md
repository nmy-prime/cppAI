# cppAI

一个使用 C++ 和 WinHTTP 调用 OpenAI Chat Completions 兼容接口的 Windows 控制台示例。项目将用户问题发送到接口，并输出 `choices[0].message.content` 中的回答。

## 功能与限制

- 使用 Windows 原生 WinHTTP，无需额外 HTTP 库。
- 从 `OPENAI_API_KEY` 环境变量读取密钥，不会把密钥写入配置文件。
- 请求和响应使用 JSON（[nlohmann/json](https://github.com/nlohmann/json)）。
- 当前示例只发送一条用户消息；问题文本在 `cppaAI/cppAI.cpp` 中的 `ai.ask(...)` 调整。
- 仅支持返回 Chat Completions 格式 `choices[0].message.content` 的服务端。

> 该项目使用 `POST /v1/chat/completions`、`Authorization: Bearer ...` 以及 `messages` 请求格式；这是 OpenAI 官方文档展示的 Chat Completions 调用方式。详见 [OpenAI API 文档](https://developers.openai.com/api/docs/guides/prompt-engineering)。

## 环境要求

- Windows 10/11
- Visual Studio 2022（C++ 桌面开发工作负载）
- Windows SDK
- 可访问所配置 API 地址的网络

## 配置

1. 在 `cppaAI` 项目目录中复制示例配置文件：

   ```powershell
   Copy-Item .\cppaAI\config.example.json .\cppaAI\config.json
   ```

2. 编辑 `cppaAI/config.json`：

   ```json
   {
     "api_url": "https://api.openai.com/v1/chat/completions",
     "model": "替换为你的可用模型"
   }
   ```

   也可以填写提供 Chat Completions 兼容接口的其他服务地址和模型名。

3. 设置 API 密钥。仅对当前 PowerShell 窗口生效：

   ```powershell
   $env:OPENAI_API_KEY = "你的_API_密钥"
   ```

   需要持久保存到当前 Windows 用户环境变量时：

   ```powershell
   [Environment]::SetEnvironmentVariable("OPENAI_API_KEY", "你的_API_密钥", "User")
   ```

   设置持久变量后，请重新打开 Visual Studio 或终端。不要将 API 密钥提交到 Git 仓库。

## 构建与运行

### Visual Studio

打开 `cppaAI.sln`，选择 `Debug | x64`，然后执行“生成解决方案”。构建后配置文件会自动复制到输出目录。

### 命令行

在仓库根目录运行：

```powershell
msbuild .\cppaAI.sln /t:Build /p:Configuration=Debug /p:Platform=x64
.\x64\Debug\cppAI.exe
```

如果 `msbuild` 不在 `PATH`，请使用 Visual Studio 的“Developer PowerShell”，或改用本机 MSBuild 的完整路径。

## 常见问题

| 输出 | 含义与处理方式 |
| --- | --- |
| `API Key不存在` | 当前进程没有读取到 `OPENAI_API_KEY`。设置后重新启动 Visual Studio/终端。 |
| `Unable to load config.json` | 检查输出目录是否存在 `config.json`，并确认 JSON 格式有效。 |
| `Unauthorized` / HTTP 401 | API 密钥无效、缺失或没有目标模型的访问权限。 |
| `Too Many Requests` / HTTP 429 | 达到速率或额度限制。 |
| `WinHttpSendRequest failed: 12002` | 网络连接超时。先运行 `Test-NetConnection api.openai.com -Port 443`；若失败，请检查网络、IPv6 路由、防火墙或 WinHTTP 代理设置。 |
| `Response missing answer` | 服务端响应不是本项目所需的 Chat Completions 格式，或没有生成文本内容。 |

浏览器能通过代理访问网络时，WinHTTP 不一定会沿用浏览器的代理。若程序仍超时，请确认 Windows 的 WinHTTP 代理和网络路由均可访问所配置的 API 主机。

## 项目结构

```text
cppaAI.sln
├─ cppaAI/
│  ├─ cppAI.cpp            # 程序入口与示例问题
│  ├─ AIClient.*           # Chat Completions 请求/响应处理
│  ├─ HttpClient.*         # WinHTTP 请求封装
│  ├─ Config.*             # JSON 配置加载
│  ├─ config.example.json  # 可提交的配置模板
│  └─ config.json          # 本地配置（已忽略）
└─ README.md
```
