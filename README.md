
# STM32 Blue Pill - Système de Contrôle d'Accès RFID

> **Corentin** et **Khalil**

Ce projet est une application embarquée pour le microcontrôleur **STM32F103C8T6** ("Blue Pill"). Il permet de lire l'ID d'un badge et d'afficher le résultat sur un écran.

Le projet est développé en **C** avec la librairie **STM32 HAL**, en utilisant un flux de travail moderne sous Linux (Ubuntu) avec **VS Code**, **CMake** et **STM32CubeMX**.

## Fonctionnalités

* **Lecture RFID (SPI) :** Détection de cartes et lecture de l'ID.
* **Affichage OLED (I2C) :** Affichage graphique de l'ID de la carte sur un écran 128x64.

## Matériel Requis

| Composant | Description | Protocole |
| --- | --- | --- |
| **STM32F103C8T6** | Carte de développement "Blue Pill" | - |
| **ST-LINK V2** | Dongle de programmation | SWD |
| **RFID-RC522** | Module lecteur de cartes | SPI (3.3V) |
| **OLED 0.96"** | Écran compatible SSD1306 (128x64) | I2C |
| **Câbles Dupont** | Pour les connexions | - |

## Schéma de Câblage (Pinout)

### 1. Module RFID-RC522 (SPI1)

| Pin RC522 | Pin STM32 | Fonction |
| --- | --- | --- |
| **SDA (SS)** | **PA4** | Chip Select |
| **SCK** | **PA5** | SPI Clock |
| **MISO** | **PA6** | Master In Slave Out |
| **MOSI** | **PA7** | Master Out Slave In |
| **RST** | **PB0** | Reset |
| **3.3V** | **3.3V** | Alimentation |
| **GND** | **GND** | Masse |
| **IRQ** | *Non connecté* | Interruption |

### 2. Écran OLED SSD1306 (I2C1)

| Pin OLED | Pin STM32 | Fonction |
| --- | --- | --- |
| **SCL** | **PB6** | I2C Clock |
| **SDA** | **PB7** | I2C Data |
| **VCC** | **5V**  | Alimentation |
| **GND** | **GND** | Masse |

### 3. Programmateur ST-LINK V2 (SWD)

| Pin ST-Link | Pin STM32 |
| --- | --- |
| **SWCLK** | **DCLK** |
| **SWDIO** | **DIO** |
| **GND** | **GND** |
| **3.3V** | **3.3V** |

---

## Installation de l'Environnement (Ubuntu/Linux)

Ce projet n'utilise pas l'IDE propriétaire STM32CubeIDE, mais une chaine de compilation open-source.

### 1. Pré-requis

#### Linux

Installer les outils nécessaires (Compilateur, Debugger, Build system) :

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build gcc-arm-none-eabi gdb-multiarch openocd
```

#### Windows

Pour l'installation sur Windows, voir l'annexe en bas.

### 2. Extensions VS Code recommandées

* **C/C++ Extension Pack** (Microsoft)
* **CMake Tools** (Microsoft)
* **Cortex-Debug** (Marus25)

### 3. Clonage et Configuration

```bash
git clone https://github.com/alCocool/stm32-rfid-oled.git
cd stm32-rfid-oled
```

Ouvrez le dossier avec VS Code. L'extension CMake devrait détecter automatiquement le projet. Sélectionnez le kit **GCC for arm-none-eabi** (si demandé).

---

## Compilation et Flash

### Sauvegarder

### Clean & Reconfigure (Important quand on change beaucoup de fichiers)

* `Ctrl + Shift + P` *CMake: Delete Cache and Reconfigure*  

(Des fois, il peut être utile de supprimer le dossier build pour le reconstruire.)

### Compiler le projet

* Cliquez sur le bouton **Build** dans la barre du bas de VS Code.

### Flasher et Débugger

Ce projet inclut un correctif pour les puces "Clones" souvent vendues comme des STM32.

1. Assurez-vous que le fichier `bluepill_fix.cfg` est présent à la racine.
2. Allez dans l'onglet **Run and Debug** de VS Code.
3. Lancez la configuration **"Cortex Debug"** (Flèche verte), assurez-vous d'avoir le fichier `.vscode/launch.json`.

VS Code va lancer OpenOCD, effacer la puce, envoyer le programme, et s'arrêter (mettre en pause) sur la première ligne du main(). Pour lancer le programme en continu, appuie sur le bouton "Continue" (la flèche de lecture bleue dans la barre flottante de débug en haut) ou touche F5.

*Note : Si vous utilisez une puce STM32 originale, modifiez le fichier `launch.json` pour retirer la référence à `bluepill_fix.cfg`.*

---

## Structure du Projet

Voici les fichiers qui ont été modifiés par rapport à la configuration initiale de STM32CubeMX.

```
.
├── .vscode
│   └── launch.json      # Configuration de débogage pour VS Code
├── Core
│   ├── Inc
│   │   ├── main.h       # Configuration globale
│   │   ├── rc522.h      # Driver RFID (Header)
│   │   └── ssd1306.h    # Driver OLED (Header)
│   └── Src
│       ├── main.c       # Logique principale (Boucle infinie)
│       ├── rc522.c      # Implémentation du protocole RFID
│       └── ssd1306.c    # Implémentation graphique OLED
├── bluepill_fix.cfg     # Patch OpenOCD pour les clones STM32
├── CMakeLists.txt       # Configuration de compilation
├── MonProjet.ioc        # Fichier de configuration STM32CubeMX
└── README.md
```

---

## Annexes

### Configuration STM32CubeMX

Pour régénérer le projet `.ioc` à partir de zéro :

**1. MCU Selector**

* **Puce :** STM32F103C8T6

**2. System Core**

* **SYS :** Debug  **Serial Wire**
* **RCC :** HSE  **Crystal/Ceramic Resonator**

**3. Clock Configuration**

* **HCLK (MHz) :** 72 (Sélectionner HSE + PLLCLK)

**4. Connectivity**

* **I2C1 (OLED) :** I2C  **Fast Mode** (400kHz)
* **SPI1 (RFID) :** Full-Duplex Master, **Prescaler 16**, CPOL Low, CPHA 1 Edge.

**5. GPIO Settings**
| Pin | Mode | Label | Note |
| :--- | :--- | :--- | :--- |
| **PA4** | Output | `RFID_CS` | **Level: High** |
| **PB0** | Output | `RFID_RST`| - |
| **PC13**| Output | `LED` | - |

**6. Project Manager**

* **Toolchain / IDE :** CMake
* **Code Generator :** Cocher "Generate peripheral initialization as a pair of '.c/.h' files"

---

### .vscode/launch.json

```
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Cortex Debug",
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceFolder}/build/Debug/MonProjetSTM32.elf",
            "request": "launch",
            "type": "cortex-debug",
            "runToEntryPoint": "main",
            "servertype": "openocd",
            "configFiles": [
                // exemple linux : "/usr/share/openocd/scripts/interface/stlink.cfg",
                // exmple windows : "C:/OpenOCD/openocd/scripts/interface/stlink.cfg",
                 "/usr/share/openocd/scripts/interface/stlink.cfg",
                "${workspaceFolder}/bluepill_fix.cfg"
            ]
        }
    ]
}
```
---

### Installation Manuelle sur Windows

Téléchargez et installez les outils suivants dans cet ordre exact :

### 1. Le Compilateur (Arm GNU Toolchain)

* **Lien :** [Arm GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
* **Fichier :** `arm-gnu-toolchain-xw.y.z-mingw-w64-i686-arm-none-eabi.exe` (Version Windows 32-bit installer).
* **IMPORTANT :** À la fin de l'installation, **cochez la case** : *"Add path to environment variable"*.

### 2. CMake (Générateur de build)

* **Lien :** [CMake Download](https://cmake.org/download/)
* **Fichier :** `cmake-x.x.x-windows-x86_64.msi`
* **IMPORTANT :** Pendant l'installation, choisissez : *"Add CMake to the system PATH for all users"*.

### 3. Ninja (Outil de build rapide)

* **Lien :** [Ninja GitHub Releases](https://github.com/ninja-build/ninja/releases)
* **Fichier :** `ninja-win.zip`
* **Installation :**
1. Dézippez le fichier. Vous obtenez `ninja.exe`.
2. **Astuce rapide :** Copiez ce fichier `ninja.exe` et collez-le dans le dossier `bin` de l'installation de CMake (par ex: `C:\Program Files\CMake\bin`). Comme ce dossier est déjà dans le PATH, Ninja sera reconnu tout de suite.

### 4. OpenOCD (Débogueur)

* **Lien :** [xPack OpenOCD Releases](https://github.com/xpack-dev-tools/openocd-xpack/releases)
* **Fichier :** `xpack-openocd-x.x.x-win32-x64.zip`
* **Installation :**
1. Décompressez le dossier à la racine de `C:` (ex: `C:\OpenOCD`).
2. **Ajout au PATH :**
* Tapez "Variables d'environnement" dans Windows.
* Dans "Variables système", modifiez `Path`.
* Ajoutez le chemin vers le dossier `bin` d'OpenOCD (ex: `C:\OpenOCD\bin`).

### 5. Driver ST-LINK

* **Lien :** [STSW-LINK009](https://www.st.com/en/development-tools/stsw-link009.html) (Nécessite une adresse email).
* **Installation :** Dézippez et lancez `dpinst_amd64.exe`.
