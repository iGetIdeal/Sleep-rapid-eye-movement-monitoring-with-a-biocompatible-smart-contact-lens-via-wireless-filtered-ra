import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, ConfusionMatrixDisplay

# --- Configuration ---
# Path to the original Excel workbook containing the complete dataset.
# This workbook is not included in the repository.
INPUT_FILENAME = r'data/full_dataset.xlsx'

# Additive Gaussian noise standard deviations for test-set features.
# These values are applied in each feature's original numeric units.
NOISE_LEVEL_FOR_LOSS = 0.5
NOISE_LEVEL_FOR_FREQ = 0.5

# Proportion of the workbook assigned to the test split.
TEST_SIZE = 0.25
# Fixed seed for the train/test split and generated noise.
RANDOM_STATE = 42

# Display names for labels 1, 2, 3, and 4, respectively.
# Confirm this mapping against the study protocol before interpreting results.
CLASS_LABELS = ['Up', 'Down', 'Left', 'Right']


def realistic_evaluation():
    """Train a random forest and evaluate it on a noise-perturbed test split."""
    print("==== Starting noisy-test evaluation ====")

    # Load the source workbook.
    try:
        print(f"Reading input file: '{INPUT_FILENAME}'...")
        df = pd.read_excel(INPUT_FILENAME)
    except FileNotFoundError:
        print(f"Error: input file '{INPUT_FILENAME}' was not found. Check the path.")
        return

    # Split the data into stratified training and test sets.
    X = df.drop('Label', axis=1)
    y = df['Label']

    X_train, X_test, y_train, y_test = train_test_split(
        X, y,
        test_size=TEST_SIZE,
        random_state=RANDOM_STATE,
        stratify=y
    )
    print(f"Split complete. Training rows: {len(X_train)}; test rows: {len(X_test)}.")

    # Train the classifier on the unmodified training data.
    print("\nTraining the random forest on the unmodified training set...")
    model = RandomForestClassifier(n_estimators=100, random_state=RANDOM_STATE)
    model.fit(X_train, y_train)
    print("Training complete.")

    # Add independent Gaussian noise to test features.
    print("\nAdding noise to test features...")
    X_test_noisy = X_test.copy()
    np.random.seed(RANDOM_STATE)

    loss_cols = [col for col in X_test.columns if 'Loss' in col]
    for col in loss_cols:
        noise = np.random.normal(0, NOISE_LEVEL_FOR_LOSS, len(X_test_noisy))
        X_test_noisy[col] += noise

    if NOISE_LEVEL_FOR_FREQ > 0:
        freq_cols = [col for col in X_test.columns if 'Freq' in col]
        for col in freq_cols:
            noise = np.random.normal(0, NOISE_LEVEL_FOR_FREQ, len(X_test_noisy))
            X_test_noisy[col] += noise

    print("Noise addition complete.")

    # Evaluate the trained classifier on the noise-perturbed test set.
    print("\nEvaluating on the noise-perturbed test set...")
    y_pred = model.predict(X_test_noisy)
    accuracy = accuracy_score(y_test, y_pred)
    print("\n--- Evaluation results ---")
    print(f"Accuracy on the noise-perturbed test set: {accuracy:.4f} ({accuracy:.2%})")

    if accuracy == 1.0:
        print("Note: accuracy remains 100%. Verify that this result is expected.")
        print("If appropriate for the study protocol, consider evaluating other noise levels.")

    # Plot the confusion matrix.
    print("\nGenerating the confusion matrix...")
    try:
        sorted_labels = sorted(y_test.unique())
        cm = confusion_matrix(y_test, y_pred, labels=sorted_labels)
        disp = ConfusionMatrixDisplay(
            confusion_matrix=cm,
            display_labels=CLASS_LABELS
        )

        fig, ax = plt.subplots(figsize=(8, 8))
        disp.plot(ax=ax, cmap='Blues', xticks_rotation='vertical')
        ax.set_title('Confusion Matrix (Noise-Perturbed Test Data)')
        plt.tight_layout()
        print("Confusion matrix generated.")
        plt.show()

    except Exception as exc:
        print(f"Error while plotting the confusion matrix: {exc}")

    print("\n==== Evaluation finished ====")


if __name__ == '__main__':
    realistic_evaluation()