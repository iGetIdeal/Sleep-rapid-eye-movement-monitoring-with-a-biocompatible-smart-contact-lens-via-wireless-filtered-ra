
import os
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, ConfusionMatrixDisplay

# --- 1. Configuration Section ---
# =======================================================================
# [REQUIRED] Paths to the training and testing datasets.
# Ensure these match the output of your data splitting script.
# We recommend placing your datasets in a 'data/' directory within your repository.
TRAIN_FILENAME = './data/train_set.csv'
TEST_FILENAME = './data/test_set.csv'

# [IMPORTANT] Define the class labels to display in the confusion matrix.
# Modify these according to your actual label meanings.
# E.g., if 0 is 'Idle' and 1-4 are directions, ensure the order matches 
# the numerical sorting of your labels (0, 1, 2, 3, 4).
CLASS_LABELS = ['Up', 'Down', 'Left', 'Right'] 
# =======================================================================


def train_and_evaluate(train_file, test_file, class_labels):
    """
    Load data, train a Random Forest model, and evaluate its accuracy 
    and confusion matrix.
    """
    print("==== Model Training and Evaluation Script Started ====")
    
    try:
        print("Loading training and testing datasets...")
        # Note: If your original data is in Excel format (.xlsx), 
        # use pd.read_excel() instead of pd.read_csv()
        train_df = pd.read_csv(train_file)
        test_df = pd.read_csv(test_file)
    except FileNotFoundError as e:
        print(f"Error: Data file not found - '{e.filename}'.")
        print("Please ensure your dataset files are placed in the correct directory.")
        return

    # Separate features (X) and labels (y)
    # Assumes the target variable is named 'Label'
    X_train = train_df.drop('Label', axis=1)
    y_train = train_df['Label']
    
    X_test = test_df.drop('Label', axis=1)
    y_test = test_df['Label']
    
    print("Data loaded successfully. Starting to train the Random Forest model...")

    # --- 2. Model Training ---
    # Initialize the Random Forest Classifier
    # n_estimators: number of trees in the forest
    # random_state: ensures reproducibility
    model = RandomForestClassifier(n_estimators=100, random_state=42, oob_score=True)
    
    # Fit the model using the training data
    model.fit(X_train, y_train)
    
    print("Model training completed.")
    print(f"Out-of-Bag (OOB) Score: {model.oob_score_:.4f} "
          "(Serves as an approximate evaluation of model generalization)")

    # --- 3. Model Evaluation ---
    print("\nEvaluating on the testing set...")
    # Predict labels for the testing set
    y_pred = model.predict(X_test)
    
    # Calculate and print accuracy
    accuracy = accuracy_score(y_test, y_pred)
    print(f"Model Accuracy on Test Set: {accuracy:.4f} ({accuracy:.2%})")

    # --- 4. Plot Confusion Matrix ---
    print("Generating confusion matrix...")
    try:
        # Get unique labels and sort them to ensure correspondence with class_labels
        sorted_labels = sorted(y_test.unique())

        cm = confusion_matrix(y_test, y_pred, labels=sorted_labels)
        
        # Visualize using ConfusionMatrixDisplay
        disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=class_labels)
        
        fig, ax = plt.subplots(figsize=(8, 8))
        disp.plot(ax=ax, cmap='Blues', xticks_rotation='vertical')
        
        ax.set_title('Confusion Matrix - Eye Movement Directions')
        plt.tight_layout()
        
        # Save the figure to the current directory
        plt.savefig('confusion_matrix.png', dpi=300)
        print("Confusion matrix plot generated and saved as 'confusion_matrix.png'.")
        
        plt.show()

    except Exception as e:
        print(f"An error occurred while plotting the confusion matrix: {e}")
        print("Please check if the number and content of 'CLASS_LABELS' "
              "exactly match the 'Label' column in your dataset.")

    print("\n==== Script Execution Finished ====")


if __name__ == '__main__':
    train_and_evaluate(TRAIN_FILENAME, TEST_FILENAME, CLASS_LABELS)
