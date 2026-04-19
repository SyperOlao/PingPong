# Mini engine

Изначально проект назывался`PingPong` и он уже давно перестал быть только Pong-игрой. Сейчас это небольшой игровой движок на `C++20` и `DirectX 11`, внутри которого есть общее `Core`, единая точка входа, общие сервисы приложения и несколько отдельных мини-игр/демо, работающих поверх одной базы.

Проект удобно воспринимать так:

- `Core` отвечает за окно, цикл приложения, ввод, рендер, аудио, ресурсы, UI и базовые игровые системы.
- `Game/*` содержит независимые игры и технодемо, которые подключаются через общий интерфейс `IGame`.
- `main.cpp` выбирает, что именно запускать в текущей сборке.

---

## Содержание

- [Что сейчас умеет проект](#что-сейчас-умеет-проект)
- [Архитектура](#архитектура)
- [Как это работает](#как-это-работает)
- [Мини-игры и демо](#мини-игры-и-демо)
- [Launch](#launch)
- [Installation third-party libraries](#installation-third-party-libraries)
- [Скриншоты](#скриншоты)
- [Структура проекта](#структура-проекта)

---

## Что сейчас умеет проект

- Запускает разные игры через единое приложение и общий интерфейс `IGame`.
- Имеет 2D- и 3D-рендеринг на `DirectX 11`.
- Даёт общие сервисы через `AppContext`, чтобы игры работали поверх одного ядра.
- Поддерживает UI-элементы, аудио, загрузку моделей и ассетов.
- Содержит базовую ECS-подобную сцену для 3D-режимов: сущности, компоненты и системы.

Если коротко: это уже не "один Pong", а песочница/движок, где можно собирать отдельные игровые режимы на общей инфраструктуре.

---

## Архитектура

### `Core/App`

Слой приложения:

- `Application` создаёт окно, графику, ввод, аудио, ресурсы и запускает главный цикл.
- `IGame` задаёт единый контракт для любой игры: `Initialize`, `Update`, `Render`, `Shutdown`.
- `AppContext` собирает доступ к сервисам платформы, ввода, графики, UI, аудио и ассетов в одном объекте.

Именно поэтому `Pong`, `SolarSystem`, `Katamari` и `LightingTest` можно запускать через одну и ту же оболочку.

### `Core/Graphics`

Графическое ядро проекта:

- `GraphicsDevice` поднимает `Direct3D 11`.
- `ShapeRenderer2D` рисует 2D-примитивы и используется UI/оверлеями.
- `PrimitiveRenderer3D` и `ModelRenderer` нужны для 3D-объектов и моделей.
- `FrameRenderer` и пайплайн в `Core/Graphics/Rendering/Pipeline` управляют проходами рендера.
- В проекте уже есть задел под forward/deferred rendering, освещение и тени.

### `Core/Input`, `Core/Audio`, `Core/Assets`

- `InputSystem` и `RawInputHandler` обрабатывают клавиатуру и мышь.
- `AudioSystem` работает через `DirectXTK Audio` и даёт one-shot/loop/instances для звука.
- `AssetCache` и `AssetPathResolver` отвечают за загрузку моделей, текстур и других ассетов.

### `Core/Gameplay`

Это базовый 3D gameplay-слой для игр, которым мало просто "нарисовать объект":

- `Scene` хранит сущности, компоненты и список систем.
- Компоненты включают `TransformComponent`, `ModelComponent`, `VelocityComponent`, `SphereColliderComponent`, `BoxColliderComponent`, `TagComponent`, `AttachmentComponent`.
- Системы вроде `TransformSystem`, `VelocityIntegrationSystem`, `CollisionSystem`, `RenderSystem` обновляют сцену каждый кадр.

По сути это лёгкая ECS-архитектура под текущие задачи проекта.

---

## Как это работает

### 1. Выбор игры

В [main.cpp](/Users/syper_olao/Desktop/program/programs/Python/PingPong/main.cpp) через `enum class DemoType` выбирается, какая игра будет создана:

- `Pong`
- `SolarSystem`
- `Katamari`
- `LightingTest`

Дальше создаётся соответствующий класс игры, реализующий `IGame`.

### 2. Запуск приложения

`Application`:

- создаёт окно;
- инициализирует графику, ввод, UI, аудио и ассеты;
- собирает всё это в `AppContext`;
- вызывает у выбранной игры `Initialize()`;
- запускает цикл `Update()` + `Render()`.

### 3. Работа кадра

Каждый кадр схема примерно такая:

1. приложение обновляет таймер и ввод;
2. активная игра получает `Update(context, deltaTime)`;
3. игра обновляет свою логику;
4. приложение и игра рисуют кадр;
5. цикл повторяется до закрытия окна.

За счёт этого новые мини-игры можно добавлять без переписывания всего приложения.

---

## Мини-игры и демо

### Pong

Классическая 2D-игра, с которой проект когда-то начинался, но теперь она стала одной из игр внутри движка.

Что есть:

- главное меню и экран настроек;
- режим `Player vs Player`;
- режим `Player vs Bot`;
- выбор сложности;
- матчевые правила;
- HUD, звук, музыка и экран завершения матча.

Основные файлы:

- `Game/Pong/PongGame.*`
- `Game/Pong/PongScene.*`
- `Game/Pong/UI/*`
- `Game/Pong/Systems/*`
- `Game/Pong/Entities/*`

#### Screenshots
The main menu
![img.png](Images/img4.png)

Gameplay
![img.png](Images/img3.png)


### SolarSystem

Небольшое 3D-демо солнечной системы с орбитами, камерой и интерактивной панелью параметров.

Что есть:

- `FPS`-камера и `Orbit`-камера;
- переключение режима камеры;
- отрисовка орбит и небесных тел;
- UI-панель с настройкой скорости вращения, скорости орбит, радиусов и эксцентриситета;
- фоновый космический рендер и аудио для движения.

Управление в текущей версии:

- `F1` - FPS camera
- `F2` - Orbit camera
- `P` - переключение режима проекции
- `Tab` - показать/скрыть панель настроек
- `WASD`, стрелки, `RMB` - перемещение и обзор

#### Screenshots
![img.png](Images/img5.png)

### Katamari

3D-мини-игра с катящимся шаром, подбором объектов и камерой сопровождения.

Что есть:

- сцена на базе `Core/Gameplay/Scene`;
- набор систем для движения, столкновений, поглощения объектов и визуального роста шара;
- follow-camera;
- HUD со статистикой;
- перезапуск уровня и debug-режим для коллизий.

Управление в текущей версии:

- `WASD` - движение
- `Space` - прыжок
- `R` - рестарт уровня
- `F3` - debug draw коллизий
- `RMB` - управление камерой

### LightingTest

Компактное техническое 3D-демо для проверки моделей, материалов, освещения и камеры.

Подходит как полигон для:

- проверки базового 3D-рендера;
- отладки света и материалов;
- быстрой валидации камеры и сцены без сложной игровой логики.

Управление:

- `WASD` - движение камеры
- стрелки - поворот
- `RMB` - обзор мышью

---

## Launch

Сейчас проект собирается как одно приложение, а конкретная игра выбирается в [main.cpp](/Users/syper_olao/Desktop/program/programs/Python/PingPong/main.cpp).

Чтобы переключить мини-игру, поменяй значение у `demo`:

```cpp
constexpr auto demo = DemoType::Katamari;
```

Доступные варианты:

```cpp
DemoType::Pong
DemoType::SolarSystem
DemoType::Katamari
DemoType::LightingTest
```

После этого можно собирать и запускать проект через `CLion`/`CMake`.

---

## Installation third-party libraries
Install vcpkg in your pc
```bash
git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
cd C:\dev\vcpkg
.\bootstrap-vcpkg.bat
```
Next you should open your project folder in administration console
```bash
cd <PATH_TO_THE_PROJECT>\PingPong
set "VCPKG_ROOT=C:\dev\vcpkg"
set "PATH=%VCPKG_ROOT%;%PATH%"

vcpkg version

vcpkg new --application
vcpkg add port directxmath
vcpkg add port directxtk
```
Then add to Clion this settings
``-DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows``

![img.png](Images/img.png)

and switch to Visual Studio in toolchain

![img.png](Images/img2.png)

---

## Структура проекта

```text
PingPong/
├── Core/          # Общее ядро движка
│   ├── App/       # Приложение, цикл, AppContext, IGame
│   ├── Graphics/  # DX11, 2D/3D рендер, пайплайн, камеры
│   ├── Gameplay/  # Scene, компоненты, системы, коллизии
│   ├── Input/     # Клавиатура, мышь, raw input
│   ├── Audio/     # Звук и музыкальные лупы
│   ├── Assets/    # Кэш и резолв ассетов
│   └── UI/        # Шрифты, кнопки, switcher, widgets
├── Game/
│   ├── Pong/         # 2D Pong
│   ├── SolarSystem/  # 3D solar system demo
│   ├── Katamari/     # 3D rolling-ball mini game
│   └── LightingTest/ # Техническое демо освещения
├── Images/        # Скриншоты для README
├── main.cpp       # Выбор активной игры
└── CMakeLists.txt # Сборка проекта
```


---

## Игры
Реализованные игры на данный момент



## Итог

Если совсем кратко, проект устроен так:

- `Core` даёт общий фундамент;
- каждая игра живёт в своём модуле внутри `Game`;
- все режимы используют единый жизненный цикл через `IGame`;
- новые мини-игры можно добавлять в существующий каркас без полной переделки проекта.

Именно поэтому репозиторий уже логичнее воспринимать не как "ещё один Pong", а как собственный компактный игровой framework с набором игровых прототипов.
