# Sleep rapid eye movement monitoring with a biocompatible smart contact lens via wireless filtered radio-frequency coupling

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![Python 3.8+](https://img.shields.io/badge/python-3.8+-blue.svg)](https://www.python.org/downloads/)

This repository contains the machine learning source code and data processing scripts for our paper:

> **"Sleep rapid eye movement monitoring with a biocompatible smart contact lens via wireless filtered radio-frequency coupling"** (In submission).

> **Scope.** The supplied materials represent two related but separate parts of the experimental workflow: (1) a random-forest evaluation using four-channel frequency/loss features, and (2) Arduino control of a two-axis eye model using prerecorded gaze values. 

## Repository structure

```text
.  
├── README.md  
├── data/  
│   ├── train_set.csv  
│   └── test_set.csv  
├── machine_learning/  
│   └── random_forest_model.py  
└── arduino/  
    ├── Code_DRYAD_4direction_REM.ino   
    └── Code_DRYAD_4direction_REM.ino  
```

## Machine-learning data and evaluation

Across the two CSV files, the dataset contains 800 observations: 640 in the training split and 160 in the test split. Both splits are class-balanced, with 160 training and 40 test observations for each label (1–4).

The feature columns are:

- `Ch1_Freq`, `Ch1_Loss`
- `Ch2_Freq`, `Ch2_Loss`
- `Ch3_Freq`, `Ch3_Loss`
- `Ch4_Freq`, `Ch4_Loss`
- `Label` (target)

Each row in `train_set.csv` and `test_set.csv` represents one eye-movement measurement acquired using a vector network analyzer (VNA), comprising resonant-signal data from four channels. For each channel, `Freq` denotes the resonance frequency of the measured signal, reported in hertz (Hz), and `Loss` denotes the corresponding return loss, reported in decibels (dB). `Label` indicates the eye-movement direction: `1 = Up`, `2 = Down`, `3 = Left`, and `4 = Right`.

### Python dependencies

The evaluation script imports:

- Python 3.8
- `pandas`
- `numpy`
- `matplotlib`
- `scikit-learn`
- An Excel reader supported by pandas (for example, `openpyxl`)

Install the packages in an isolated environment, then run the script after setting a valid workbook path:

```bash
python -m pip install pandas numpy matplotlib scikit-learn openpyxl  
python machine_learning/random_forest_noisy_test_evaluation.py  
```

## Arduino eye-model control

Both sketches target an Arduino Nano-style board and an Adafruit PCA9685 PWM driver. The Arduino IDE's Library Manager should be used to install the Adafruit PWM Servo Driver library. They store prerecorded gaze values in program memory (`PROGMEM`) and control horizontal and vertical axes through servo PWM outputs. The sketches use different command representations and timing settings; they are alternative implementations, not interchangeable data formats.

已根据截图中的文件名后缀明确区分两种状态，并更新 README：

### Arduino-Based Eye-Movement Simulation

The two Arduino sketches reproduce prerecorded eye movements under two behavioral conditions: REM sleep and wakefulness. Their original filename suffixes identify the conditions: `Code_DRYAD_4direction_REM.ino` is the REM-sleep simulation, and `Code_DRYAD_4direction_weak.ino` is the wakefulness simulation. In this repository, these sketches are organized as follows:

The eye model is controlled by an Arduino microcontroller connected to a PCA9685 PWM driver. The Arduino sends control commands through the PCA9685 to drive the eye-model actuators. This description identifies the experimental condition represented by each sketch and does not interpret or rely on the source code's internal classification logic.

### Hardware and safety

Before connecting servos, verify the PCA9685 channel assignments, servo direction, pulse range, mechanical travel, and power requirements for the specific hardware. Use an appropriate external servo supply where required, share ground with the controller, and test with the mechanism unloaded or mechanically constrained. The angle-to-pulse conversion constants and base pulse values are hardware-specific and may require calibration. Never assume that values safe for one eye model are safe for another.

## Citation

This work is currently in submission.

```text
Author(s). (Year). Article title. Journal or conference, volume(issue), pages. DOI  
```