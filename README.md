# Eye-Machine Interaction (EMI) Direction Classification

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![Python 3.8+](https://img.shields.io/badge/python-3.8+-blue.svg)](https://www.python.org/downloads/)

This repository contains the machine learning source code and data processing scripts for our paper:

> **"[Insert Your Paper Title Here]"** (Currently under review / Published in [Journal/Conference Name]).

## 📌 Overview

This project utilizes a Random Forest (RF) classifier to interpret and classify eye movements into distinct directional commands (e.g., Up, Down, Left, Right). It is designed to facilitate research in Eye-Machine Interaction (EMI) interfaces by providing a robust, interpretable machine learning pipeline.

## 📂 Repository Structure

```text
.
├── data/
│   ├── train_set.csv        # Training dataset (Please upload your sample data here)
│   └── test_set.csv         # Testing dataset
├── eye_movement_random_forest_classifier.py        # Main script for training and evaluating the RF model
├── requirements.txt         # List of Python dependencies
└── README.md                # Project documentation
```

## 🚀 Getting Started

### 1. Prerequisites

Ensure you have Python 3.8 or higher installed. Clone this repository to your local machine:

```bash
git clone https://github.com/your-username/your-repo-name.git
cd your-repo-name
```

### 2. Install Dependencies

Install the required Python packages using pip:

```bash
pip install pandas scikit-learn matplotlib
```

*(Alternatively, you can run `pip install -r requirements.txt` if you create one).*

### 3. Data Preparation

The script expects the training and testing data to be in CSV format, located in the `data/` directory.

- The target variable column must be named `Label`.
- The `Label` column should contain numerical values (e.g., 0, 1, 2, 3) corresponding to the directions defined in the script.

### 4. Run the Model

Execute the main script to train the model, evaluate its accuracy, and generate the confusion matrix:

```bash
python train_evaluate.py
```

The script will output the Out-of-Bag (OOB) score, the accuracy on the test set, and automatically save a high-resolution `confusion_matrix.png` in the root directory.

## 📝 Citation

If you find this code useful for your research, please consider citing our paper:

```bibtex
@article{your_lastname2024title,
  title={Your Paper Title Here},
  author={Your Name and Co-authors},
  journal={Journal or Conference Name},
  year={2024},
  volume={xx},
  pages={xxx-xxx},
  publisher={Publisher Name}
}
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](chrome-extension://dhoenijjpgpeimemopealfcbiecgceod/LICENSE) file for details.