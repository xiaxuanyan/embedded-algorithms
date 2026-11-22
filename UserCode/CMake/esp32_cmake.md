# ESP32 (ESP-IDF) 工程结构与 CMake 笔记

## 一、标准工程目录与文件作用

从零新建一个能编译运行的 ESP-IDF 工程，**最少只需要手动写 3 个文件**：

```text
my_project/
├── CMakeLists.txt          # 1. 根目录构建脚本（工程总管）
└── main/
    ├── CMakeLists.txt      # 2. main 组件构建脚本（管理源码与头文件）
    └── main.c              # 3. 主程序（入口函数固定为 void app_main(void)）
```

### 常见自动生成 / 配置文件说明

| 文件 / 文件夹 | 作用 | 是否需要手动维护 |
| :--- | :--- | :--- |
| **`build/`** | 编译产物目录（`.o`、`.a` 及最终烧录的 `.bin`） | 自动生成，随时可删，重编即重建 |
| **`.vscode/`** | VS Code 与插件环境配置（代码跳转路径、串口号等） | 插件自动生成，不参与编译 |
| **`sdkconfig`** | 系统底层配置（芯片型号、Flash、FreeRTOS、Wi-Fi 等） | 通过 `menuconfig` 自动生成和更新 |
| **`sdkconfig.defaults`** | 默认配置模板，删掉 `sdkconfig` 重编时按此生成 | 按需手写关键默认项 |
| **`partitions.csv`** | Flash 分区表（规划 Bootloader、NVS、App、OTA 空间） | 固件超限或做 OTA 时才需要手写 |

---

## 二、两级 `CMakeLists.txt` 写法

ESP-IDF 采用**两级 CMake 构建结构**，各自职责独立。

### 1. 根目录 `CMakeLists.txt`（工程级）

只做全局声明，建好后基本不用再改：

```cmake
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my_project)
```

> **避坑：`set(COMPONENTS main)`**
> * 加上这行会开启**极简构建模式**，屏蔽绝大多数官方组件（如 `esp_wifi`、`nvs_flash`、外设驱动），仅用于加快首次全量编译。
> * **开发阶段建议删掉**：否则每次 `#include` 官方外设都会报错，必须手动往 `main/CMakeLists.txt` 里补写一长串 `REQUIRES`。
> * 删掉它**不会增大最终 `.bin` 固件体积**（链接器会自动剔除未调用的代码）。

### 2. `main/CMakeLists.txt`（组件级）

负责登记要编译的 `.c` 文件和头文件搜索路径：

```cmake
idf_component_register(
    SRCS
        "main.c"
        "gamepad_espnow/gamepad_espnow.c"
    INCLUDE_DIRS
        "."
        "gamepad_espnow"
)
```

* **`SRCS`（编译谁）**：只填 `.c` 源文件，**不填 `.h` 文件**（`.h` 不参与独立编译，预处理阶段会被直接展开到 `.c` 中）。
  * *懒人写法*：文件很多时可用 `SRC_DIRS "." "子目录"` 自动扫描目录下所有 `.c` 文件（新增文件后若未识别，执行一次 `idf.py reconfigure`）。
* **`INCLUDE_DIRS`（去哪找头文件）**：等价于 GCC 的 `-I` 参数，告诉编译器 `#include` 去哪些目录找 `.h`。

---

## 三、路径规则与头文件避坑

### 1. 相对路径规则（与终端一致）

* **起点**：所有路径均以**当前 `CMakeLists.txt` 所在的文件夹**为基准。
* **符号**：`.` 代表当前目录，`..` 代表上一级目录，路径分隔符统一用正斜杠 **`/`**（Windows 下也用 `/`，避免 `\` 被当成转义字符）。
* **注意**：`SRCS` 不会去 `INCLUDE_DIRS` 里自动找 `.c`，子文件夹里的源文件必须写清完整相对路径（如 `"sub/foo.c"`）。

### 2. 头文件搜索顺序与防重名

使用 `#include "xxx.h"` 时，编译器按以下顺序逐个查找，**找到第一个同名的立即停止**：

1. 当前 `.c` 文件所在目录
2. `INCLUDE_DIRS` 中列出的目录（按顺序）
3. ESP-IDF 官方组件与标准库目录

* **防重名**：自建头文件**不要与官方头文件重名**（例如不要起名 `esp_now.h`，否则会截胡官方头文件导致找不到官方 API）。
* **精准锁定目录**：如果想彻底避免重名串台，`INCLUDE_DIRS` 可以只留 `"."`，在代码里显式带目录名引用：`#include "子目录/xxx.h"`。

---

## 四、代码目录的 3 种组织方式

| 摆放方式 | 配置方法 | 适用场景 |
| :--- | :--- | :--- |
| **`main/` 内建子文件夹**（推荐） | 在 `main/CMakeLists.txt` 写 `"子目录/xxx.c"` | 当前工程按功能模块分类，最规范省心 |
| **`main/` 外建普通文件夹** | 在 `main/CMakeLists.txt` 用 `"../目录/xxx.c"` 向上跳一级 | 临时存放、或与 STM32 共享纯 C 协议代码 |
| **根目录 `components/`** | 每个模块自带独立 `CMakeLists.txt`，自动编译，但用到官方外设必须手写 `REQUIRES` | 跨工程复用的通用驱动或第三方库 |

---

## 五、多芯片工程与 CLion 开发备忘

* **多芯片工程（如 STM32 + ESP32）**：
  * 架构、交叉编译器和底层框架完全不同，必须在总目录下为每个芯片建**独立子目录**，各自维护自己的 `CMakeLists.txt` 和 `build/`，互不干扰。
  * 用 IDE 开发 ESP32 时，直接打开 ESP32 的**子目录**作为工作区，不要打开最外层总目录。
  * 两块芯片共用的纯 C 通信协议可放在外层 `common/` 目录，两边各自通过相对路径引用。
* **CLion 开发 ESP32**：
  * **插件方式**：在 Marketplace 搜索安装社区第三方 **`ESP-IDF`** 插件，指向本地已有的 ESP-IDF 路径，即可获得图形化 `menuconfig`、串口选择和一键编译烧录。
  * **原生方式**：在 `Settings → Toolchains` 的 Environment 中填入本地 ESP-IDF 的 `export.bat`，CMake Generator 选 `Ninja`，配合内置终端执行 `idf.py build flash monitor`。