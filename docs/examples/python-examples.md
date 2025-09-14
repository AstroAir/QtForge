# Python Examples

This collection demonstrates various Python plugin implementations and integration patterns with QtForge.

## Data Analysis Plugin

### Scientific Computing Plugin

```python
# scientific_analysis_plugin.py
import qtforge
from qtforge.core import IPlugin, PluginState, PluginError
from qtforge.communication import MessageBus
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from scipy import stats
import json
import io
import base64

class ScientificAnalysisPlugin(IPlugin):
    def __init__(self):
        super().__init__()
        self._state = PluginState.UNLOADED
        self._message_bus = None
        self._logger = qtforge.utils.get_logger(__name__)

    def name(self) -> str:
        return "ScientificAnalysisPlugin"

    def version(self) -> str:
        return "1.0.0"

    def description(self) -> str:
        return "Scientific data analysis plugin with NumPy, Pandas, and SciPy"

    def author(self) -> str:
        return "Data Science Team"

    def dependencies(self) -> list:
        return ["CorePlugin >= 1.0.0"]

    def initialize(self) -> bool:
        try:
            self._logger.info("Initializing Scientific Analysis Plugin...")

            # Initialize message bus
            self._message_bus = MessageBus.instance()

            # Subscribe to analysis requests
            self._message_bus.subscribe("analysis.statistical", self._handle_statistical_analysis)
            self._message_bus.subscribe("analysis.visualization", self._handle_visualization)
            self._message_bus.subscribe("analysis.correlation", self._handle_correlation_analysis)
            self._message_bus.subscribe("analysis.regression", self._handle_regression_analysis)

            self._state = PluginState.INITIALIZED
            self._logger.info("Scientific Analysis Plugin initialized successfully")
            return True

        except Exception as e:
            self._logger.error(f"Initialization failed: {e}")
            self._state = PluginState.ERROR
            return False

    def activate(self) -> bool:
        if self._state != PluginState.INITIALIZED:
            return False

        try:
            self._logger.info("Activating Scientific Analysis Plugin...")

            # Publish activation message
            self._message_bus.publish("plugin.events", {
                "type": "activation",
                "plugin": self.name(),
                "timestamp": qtforge.utils.current_timestamp(),
                "capabilities": [
                    "statistical_analysis",
                    "data_visualization",
                    "correlation_analysis",
                    "regression_analysis"
                ]
            })

            self._state = PluginState.ACTIVE
            self._logger.info("Scientific Analysis Plugin activated successfully")
            return True

        except Exception as e:
            self._logger.error(f"Activation failed: {e}")
            self._state = PluginState.ERROR
            return False

    def _handle_statistical_analysis(self, message):
        """Perform statistical analysis on data"""
        try:
            data = message.get("data")
            analysis_type = message.get("analysis_type", "descriptive")

            # Convert to pandas DataFrame
            df = pd.DataFrame(data)

            result = {}

            if analysis_type == "descriptive":
                result = self._descriptive_statistics(df)
            elif analysis_type == "normality":
                result = self._normality_tests(df)
            elif analysis_type == "hypothesis":
                result = self._hypothesis_tests(df, message.get("test_params", {}))
            else:
                raise ValueError(f"Unknown analysis type: {analysis_type}")

            # Publish result
            self._message_bus.publish("analysis.result", {
                "analysis_id": message.get("analysis_id"),
                "type": "statistical",
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })

        except Exception as e:
            self._logger.error(f"Statistical analysis failed: {e}")
            self._message_bus.publish("analysis.error", {
                "error": str(e),
                "analysis_type": "statistical",
                "timestamp": qtforge.utils.current_timestamp()
            })

    def _descriptive_statistics(self, df):
        """Calculate descriptive statistics"""
        numeric_cols = df.select_dtypes(include=[np.number]).columns

        stats_result = {}

        for col in numeric_cols:
            col_stats = {
                "count": int(df[col].count()),
                "mean": float(df[col].mean()),
                "median": float(df[col].median()),
                "std": float(df[col].std()),
                "var": float(df[col].var()),
                "min": float(df[col].min()),
                "max": float(df[col].max()),
                "q25": float(df[col].quantile(0.25)),
                "q75": float(df[col].quantile(0.75)),
                "skewness": float(stats.skew(df[col].dropna())),
                "kurtosis": float(stats.kurtosis(df[col].dropna()))
            }
            stats_result[col] = col_stats

        return {
            "descriptive_statistics": stats_result,
            "data_shape": df.shape,
            "missing_values": df.isnull().sum().to_dict(),
            "data_types": df.dtypes.astype(str).to_dict()
        }

    def _normality_tests(self, df):
        """Perform normality tests"""
        numeric_cols = df.select_dtypes(include=[np.number]).columns

        normality_results = {}

        for col in numeric_cols:
            data = df[col].dropna()

            if len(data) < 3:
                continue

            # Shapiro-Wilk test
            shapiro_stat, shapiro_p = stats.shapiro(data)

            # Kolmogorov-Smirnov test
            ks_stat, ks_p = stats.kstest(data, 'norm', args=(data.mean(), data.std()))

            # Anderson-Darling test
            ad_result = stats.anderson(data, dist='norm')

            normality_results[col] = {
                "shapiro_wilk": {
                    "statistic": float(shapiro_stat),
                    "p_value": float(shapiro_p),
                    "is_normal": shapiro_p > 0.05
                },
                "kolmogorov_smirnov": {
                    "statistic": float(ks_stat),
                    "p_value": float(ks_p),
                    "is_normal": ks_p > 0.05
                },
                "anderson_darling": {
                    "statistic": float(ad_result.statistic),
                    "critical_values": ad_result.critical_values.tolist(),
                    "significance_levels": ad_result.significance_level.tolist()
                }
            }

        return {"normality_tests": normality_results}

    def _handle_visualization(self, message):
        """Create data visualizations"""
        try:
            data = message.get("data")
            plot_type = message.get("plot_type", "histogram")
            plot_params = message.get("plot_params", {})

            # Convert to pandas DataFrame
            df = pd.DataFrame(data)

            # Create plot
            plt.figure(figsize=(10, 6))

            if plot_type == "histogram":
                self._create_histogram(df, plot_params)
            elif plot_type == "scatter":
                self._create_scatter_plot(df, plot_params)
            elif plot_type == "correlation_heatmap":
                self._create_correlation_heatmap(df, plot_params)
            elif plot_type == "box_plot":
                self._create_box_plot(df, plot_params)
            else:
                raise ValueError(f"Unknown plot type: {plot_type}")

            # Save plot to base64 string
            buffer = io.BytesIO()
            plt.savefig(buffer, format='png', dpi=300, bbox_inches='tight')
            buffer.seek(0)
            plot_data = base64.b64encode(buffer.getvalue()).decode()
            plt.close()

            # Publish result
            self._message_bus.publish("analysis.result", {
                "analysis_id": message.get("analysis_id"),
                "type": "visualization",
                "result": {
                    "plot_type": plot_type,
                    "plot_data": plot_data,
                    "format": "png"
                },
                "timestamp": qtforge.utils.current_timestamp()
            })

        except Exception as e:
            self._logger.error(f"Visualization failed: {e}")
            self._message_bus.publish("analysis.error", {
                "error": str(e),
                "analysis_type": "visualization",
                "timestamp": qtforge.utils.current_timestamp()
            })

    def _create_histogram(self, df, params):
        """Create histogram plot"""
        column = params.get("column")
        bins = params.get("bins", 30)

        if column and column in df.columns:
            plt.hist(df[column].dropna(), bins=bins, alpha=0.7, edgecolor='black')
            plt.title(f'Histogram of {column}')
            plt.xlabel(column)
            plt.ylabel('Frequency')
        else:
            # Plot histograms for all numeric columns
            numeric_cols = df.select_dtypes(include=[np.number]).columns
            n_cols = len(numeric_cols)

            if n_cols > 0:
                fig, axes = plt.subplots(nrows=(n_cols + 2) // 3, ncols=3, figsize=(15, 5 * ((n_cols + 2) // 3)))
                axes = axes.flatten() if n_cols > 1 else [axes]

                for i, col in enumerate(numeric_cols):
                    axes[i].hist(df[col].dropna(), bins=bins, alpha=0.7, edgecolor='black')
                    axes[i].set_title(f'Histogram of {col}')
                    axes[i].set_xlabel(col)
                    axes[i].set_ylabel('Frequency')

                # Hide unused subplots
                for i in range(n_cols, len(axes)):
                    axes[i].set_visible(False)

    def _create_correlation_heatmap(self, df, params):
        """Create correlation heatmap"""
        numeric_df = df.select_dtypes(include=[np.number])
        correlation_matrix = numeric_df.corr()

        plt.figure(figsize=(12, 8))
        sns.heatmap(correlation_matrix, annot=True, cmap='coolwarm', center=0,
                   square=True, linewidths=0.5)
        plt.title('Correlation Heatmap')
        plt.tight_layout()

    def _handle_regression_analysis(self, message):
        """Perform regression analysis"""
        try:
            data = message.get("data")
            target_column = message.get("target_column")
            feature_columns = message.get("feature_columns", [])
            regression_type = message.get("regression_type", "linear")

            # Convert to pandas DataFrame
            df = pd.DataFrame(data)

            if target_column not in df.columns:
                raise ValueError(f"Target column '{target_column}' not found in data")

            # Prepare features and target
            if not feature_columns:
                feature_columns = [col for col in df.select_dtypes(include=[np.number]).columns
                                 if col != target_column]

            X = df[feature_columns].dropna()
            y = df[target_column].dropna()

            # Align X and y indices
            common_indices = X.index.intersection(y.index)
            X = X.loc[common_indices]
            y = y.loc[common_indices]

            result = {}

            if regression_type == "linear":
                result = self._linear_regression(X, y, feature_columns, target_column)
            elif regression_type == "polynomial":
                degree = message.get("polynomial_degree", 2)
                result = self._polynomial_regression(X, y, feature_columns, target_column, degree)
            else:
                raise ValueError(f"Unknown regression type: {regression_type}")

            # Publish result
            self._message_bus.publish("analysis.result", {
                "analysis_id": message.get("analysis_id"),
                "type": "regression",
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })

        except Exception as e:
            self._logger.error(f"Regression analysis failed: {e}")
            self._message_bus.publish("analysis.error", {
                "error": str(e),
                "analysis_type": "regression",
                "timestamp": qtforge.utils.current_timestamp()
            })

    def _linear_regression(self, X, y, feature_columns, target_column):
        """Perform linear regression analysis"""
        from sklearn.linear_model import LinearRegression
        from sklearn.model_selection import train_test_split
        from sklearn.metrics import r2_score, mean_squared_error, mean_absolute_error

        # Split data
        X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

        # Fit model
        model = LinearRegression()
        model.fit(X_train, y_train)

        # Make predictions
        y_pred_train = model.predict(X_train)
        y_pred_test = model.predict(X_test)

        # Calculate metrics
        train_r2 = r2_score(y_train, y_pred_train)
        test_r2 = r2_score(y_test, y_pred_test)
        train_rmse = np.sqrt(mean_squared_error(y_train, y_pred_train))
        test_rmse = np.sqrt(mean_squared_error(y_test, y_pred_test))
        train_mae = mean_absolute_error(y_train, y_pred_train)
        test_mae = mean_absolute_error(y_test, y_pred_test)

        return {
            "model_type": "linear_regression",
            "coefficients": dict(zip(feature_columns, model.coef_.tolist())),
            "intercept": float(model.intercept_),
            "metrics": {
                "train_r2": float(train_r2),
                "test_r2": float(test_r2),
                "train_rmse": float(train_rmse),
                "test_rmse": float(test_rmse),
                "train_mae": float(train_mae),
                "test_mae": float(test_mae)
            },
            "feature_importance": dict(zip(feature_columns, np.abs(model.coef_).tolist())),
            "predictions": {
                "train": y_pred_train.tolist(),
                "test": y_pred_test.tolist()
            }
        }

# Plugin factory function
def create_plugin():
    return ScientificAnalysisPlugin()
```

## Machine Learning Plugin

### ML Model Training Plugin

```python
# ml_training_plugin.py
import qtforge
from qtforge.core import IPlugin, PluginState
from qtforge.communication import MessageBus
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestClassifier, RandomForestRegressor
from sklearn.svm import SVC, SVR
from sklearn.linear_model import LogisticRegression, LinearRegression
from sklearn.model_selection import train_test_split, cross_val_score, GridSearchCV
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score
from sklearn.metrics import mean_squared_error, r2_score
from sklearn.preprocessing import StandardScaler, LabelEncoder
import joblib
import json
import os

class MLTrainingPlugin(IPlugin):
    def __init__(self):
        super().__init__()
        self._state = PluginState.UNLOADED
        self._message_bus = None
        self._models = {}
        self._scalers = {}
        self._encoders = {}
        self._logger = qtforge.utils.get_logger(__name__)

    def name(self) -> str:
        return "MLTrainingPlugin"

    def version(self) -> str:
        return "1.0.0"

    def description(self) -> str:
        return "Machine Learning model training and prediction plugin"

    def initialize(self) -> bool:
        try:
            self._logger.info("Initializing ML Training Plugin...")

            # Initialize message bus
            self._message_bus = MessageBus.instance()

            # Subscribe to ML requests
            self._message_bus.subscribe("ml.train", self._handle_train_model)
            self._message_bus.subscribe("ml.predict", self._handle_predict)
            self._message_bus.subscribe("ml.evaluate", self._handle_evaluate_model)
            self._message_bus.subscribe("ml.save_model", self._handle_save_model)
            self._message_bus.subscribe("ml.load_model", self._handle_load_model)

            self._state = PluginState.INITIALIZED
            self._logger.info("ML Training Plugin initialized successfully")
            return True

        except Exception as e:
            self._logger.error(f"Initialization failed: {e}")
            self._state = PluginState.ERROR
            return False

    def _handle_train_model(self, message):
        """Train a machine learning model"""
        try:
            data = message.get("data")
            target_column = message.get("target_column")
            model_type = message.get("model_type", "random_forest")
            task_type = message.get("task_type", "classification")  # or "regression"
            model_params = message.get("model_params", {})
            model_id = message.get("model_id", f"{model_type}_{task_type}")

            # Convert to pandas DataFrame
            df = pd.DataFrame(data)

            # Prepare features and target
            feature_columns = [col for col in df.columns if col != target_column]
            X = df[feature_columns]
            y = df[target_column]

            # Handle categorical features
            X_processed = self._preprocess_features(X, model_id)
            y_processed = self._preprocess_target(y, task_type, model_id)

            # Split data
            test_size = message.get("test_size", 0.2)
            X_train, X_test, y_train, y_test = train_test_split(
                X_processed, y_processed, test_size=test_size, random_state=42)

            # Create and train model
            model = self._create_model(model_type, task_type, model_params)
            model.fit(X_train, y_train)

            # Store model
            self._models[model_id] = model

            # Evaluate model
            train_score = model.score(X_train, y_train)
            test_score = model.score(X_test, y_test)

            # Make predictions for detailed metrics
            y_pred_train = model.predict(X_train)
            y_pred_test = model.predict(X_test)

            # Calculate detailed metrics
            metrics = self._calculate_metrics(y_train, y_pred_train, y_test, y_pred_test, task_type)

            result = {
                "model_id": model_id,
                "model_type": model_type,
                "task_type": task_type,
                "train_score": float(train_score),
                "test_score": float(test_score),
                "metrics": metrics,
                "feature_columns": feature_columns,
                "target_column": target_column
            }

            # Add feature importance if available
            if hasattr(model, 'feature_importances_'):
                result["feature_importance"] = dict(zip(
                    feature_columns, model.feature_importances_.tolist()))

            # Publish result
            self._message_bus.publish("ml.training_result", {
                "training_id": message.get("training_id"),
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })

        except Exception as e:
            self._logger.error(f"Model training failed: {e}")
            self._message_bus.publish("ml.error", {
                "error": str(e),
                "operation": "train_model",
                "timestamp": qtforge.utils.current_timestamp()
            })

    def _preprocess_features(self, X, model_id):
        """Preprocess features (scaling, encoding)"""
        X_processed = X.copy()

        # Handle categorical columns
        categorical_columns = X.select_dtypes(include=['object']).columns
        for col in categorical_columns:
            encoder_key = f"{model_id}_{col}_encoder"
            if encoder_key not in self._encoders:
                self._encoders[encoder_key] = LabelEncoder()
                X_processed[col] = self._encoders[encoder_key].fit_transform(X[col].astype(str))
            else:
                X_processed[col] = self._encoders[encoder_key].transform(X[col].astype(str))

        # Scale numerical features
        numerical_columns = X.select_dtypes(include=[np.number]).columns
        if len(numerical_columns) > 0:
            scaler_key = f"{model_id}_scaler"
            if scaler_key not in self._scalers:
                self._scalers[scaler_key] = StandardScaler()
                X_processed[numerical_columns] = self._scalers[scaler_key].fit_transform(X[numerical_columns])
            else:
                X_processed[numerical_columns] = self._scalers[scaler_key].transform(X[numerical_columns])

        return X_processed

    def _preprocess_target(self, y, task_type, model_id):
        """Preprocess target variable"""
        if task_type == "classification" and y.dtype == 'object':
            encoder_key = f"{model_id}_target_encoder"
            if encoder_key not in self._encoders:
                self._encoders[encoder_key] = LabelEncoder()
                return self._encoders[encoder_key].fit_transform(y.astype(str))
            else:
                return self._encoders[encoder_key].transform(y.astype(str))
        return y

    def _create_model(self, model_type, task_type, params):
        """Create ML model based on type and task"""
        if task_type == "classification":
            if model_type == "random_forest":
                return RandomForestClassifier(**params)
            elif model_type == "svm":
                return SVC(**params)
            elif model_type == "logistic_regression":
                return LogisticRegression(**params)
            else:
                raise ValueError(f"Unknown classification model: {model_type}")

        elif task_type == "regression":
            if model_type == "random_forest":
                return RandomForestRegressor(**params)
            elif model_type == "svm":
                return SVR(**params)
            elif model_type == "linear_regression":
                return LinearRegression(**params)
            else:
                raise ValueError(f"Unknown regression model: {model_type}")

        else:
            raise ValueError(f"Unknown task type: {task_type}")

    def _calculate_metrics(self, y_train, y_pred_train, y_test, y_pred_test, task_type):
        """Calculate detailed metrics based on task type"""
        if task_type == "classification":
            return {
                "train_accuracy": float(accuracy_score(y_train, y_pred_train)),
                "test_accuracy": float(accuracy_score(y_test, y_pred_test)),
                "train_precision": float(precision_score(y_train, y_pred_train, average='weighted')),
                "test_precision": float(precision_score(y_test, y_pred_test, average='weighted')),
                "train_recall": float(recall_score(y_train, y_pred_train, average='weighted')),
                "test_recall": float(recall_score(y_test, y_pred_test, average='weighted')),
                "train_f1": float(f1_score(y_train, y_pred_train, average='weighted')),
                "test_f1": float(f1_score(y_test, y_pred_test, average='weighted'))
            }
        else:  # regression
            return {
                "train_mse": float(mean_squared_error(y_train, y_pred_train)),
                "test_mse": float(mean_squared_error(y_test, y_pred_test)),
                "train_rmse": float(np.sqrt(mean_squared_error(y_train, y_pred_train))),
                "test_rmse": float(np.sqrt(mean_squared_error(y_test, y_pred_test))),
                "train_r2": float(r2_score(y_train, y_pred_train)),
                "test_r2": float(r2_score(y_test, y_pred_test))
            }

    def _handle_predict(self, message):
        """Make predictions using trained model"""
        try:
            model_id = message.get("model_id")
            data = message.get("data")

            if model_id not in self._models:
                raise ValueError(f"Model '{model_id}' not found")

            model = self._models[model_id]

            # Convert to pandas DataFrame
            df = pd.DataFrame(data)

            # Preprocess features
            X_processed = self._preprocess_features(df, model_id)

            # Make predictions
            predictions = model.predict(X_processed)

            # Get prediction probabilities if available
            probabilities = None
            if hasattr(model, 'predict_proba'):
                probabilities = model.predict_proba(X_processed).tolist()

            result = {
                "model_id": model_id,
                "predictions": predictions.tolist(),
                "probabilities": probabilities,
                "input_samples": len(df)
            }

            # Publish result
            self._message_bus.publish("ml.prediction_result", {
                "prediction_id": message.get("prediction_id"),
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })

        except Exception as e:
            self._logger.error(f"Prediction failed: {e}")
            self._message_bus.publish("ml.error", {
                "error": str(e),
                "operation": "predict",
                "timestamp": qtforge.utils.current_timestamp()
            })

# Plugin factory function
def create_plugin():
    return MLTrainingPlugin()
```

## Web Scraping Plugin

### Data Collection Plugin

```python
# web_scraping_plugin.py
import qtforge
from qtforge.core import IPlugin, PluginState
from qtforge.communication import MessageBus
import requests
from bs4 import BeautifulSoup
import pandas as pd
import json
import time
import asyncio
import aiohttp
from urllib.parse import urljoin, urlparse
import re

class WebScrapingPlugin(IPlugin):
    def __init__(self):
        super().__init__()
        self._state = PluginState.UNLOADED
        self._message_bus = None
        self._session = None
        self._logger = qtforge.utils.get_logger(__name__)

    def name(self) -> str:
        return "WebScrapingPlugin"

    def version(self) -> str:
        return "1.0.0"

    def description(self) -> str:
        return "Web scraping plugin for data collection from websites"

    def initialize(self) -> bool:
        try:
            self._logger.info("Initializing Web Scraping Plugin...")

            # Initialize message bus
            self._message_bus = MessageBus.instance()

            # Create requests session
            self._session = requests.Session()
            self._session.headers.update({
                'User-Agent': 'QtForge-WebScraper/1.0'
            })

            # Subscribe to scraping requests
            self._message_bus.subscribe("scraping.single_page", self._handle_single_page_scraping)
            self._message_bus.subscribe("scraping.multiple_pages", self._handle_multiple_pages_scraping)
            self._message_bus.subscribe("scraping.table_extraction", self._handle_table_extraction)
            self._message_bus.subscribe("scraping.form_submission", self._handle_form_submission)

            self._state = PluginState.INITIALIZED
            self._logger.info("Web Scraping Plugin initialized successfully")
            return True

        except Exception as e:
            self._logger.error(f"Initialization failed: {e}")
            self._state = PluginState.ERROR
            return False

    def _handle_single_page_scraping(self, message):
        """Scrape data from a single web page"""
        try:
            url = message.get("url")
            selectors = message.get("selectors", {})
            headers = message.get("headers", {})
            delay = message.get("delay", 1)

            # Add custom headers
            session_headers = self._session.headers.copy()
            session_headers.update(headers)

            # Make request
            response = self._session.get(url, headers=session_headers)
            response.raise_for_status()

            # Parse HTML
            soup = BeautifulSoup(response.content, 'html.parser')

            # Extract data based on selectors
            extracted_data = {}
            for key, selector in selectors.items():
                elements = soup.select(selector)
                if len(elements) == 1:
                    extracted_data[key] = elements[0].get_text(strip=True)
                else:
                    extracted_data[key] = [elem.get_text(strip=True) for elem in elements]

            # Add metadata
            result = {
                "url": url,
                "data": extracted_data,
                "page_title": soup.title.string if soup.title else None,
                "status_code": response.status_code,
                "content_length": len(response.content),
                "timestamp": qtforge.utils.current_timestamp()
            }

            # Publish result
            self._message_bus.publish("scraping.result", {
                "scraping_id": message.get("scraping_id"),
                "type": "single_page",
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })

            # Respect rate limiting
            time.sleep(delay)

        except Exception as e:
            self._logger.error(f"Single page scraping failed: {e}")
            self._message_bus.publish("scraping.error", {
                "error": str(e),
                "operation": "single_page_scraping",
                "url": message.get("url"),
                "timestamp": qtforge.utils.current_timestamp()
            })

    def _handle_table_extraction(self, message):
        """Extract tables from web pages"""
        try:
            url = message.get("url")
            table_selector = message.get("table_selector", "table")
            headers = message.get("headers", {})

            # Make request
            response = self._session.get(url, headers=headers)
            response.raise_for_status()

            # Use pandas to extract tables
            tables = pd.read_html(response.content)

            # Convert tables to dictionaries
            extracted_tables = []
            for i, table in enumerate(tables):
                table_data = {
                    "table_index": i,
                    "columns": table.columns.tolist(),
                    "data": table.to_dict('records'),
                    "shape": table.shape
                }
                extracted_tables.append(table_data)

            result = {
                "url": url,
                "tables": extracted_tables,
                "table_count": len(tables),
                "timestamp": qtforge.utils.current_timestamp()
            }

            # Publish result
            self._message_bus.publish("scraping.result", {
                "scraping_id": message.get("scraping_id"),
                "type": "table_extraction",
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })

        except Exception as e:
            self._logger.error(f"Table extraction failed: {e}")
            self._message_bus.publish("scraping.error", {
                "error": str(e),
                "operation": "table_extraction",
                "url": message.get("url"),
                "timestamp": qtforge.utils.current_timestamp()
            })

# Plugin factory function
def create_plugin():
    return WebScrapingPlugin()
```

## Advanced Interface Examples (v3.2.0+)

### Service Plugin Example

```python
# service_plugin.py
import qtforge.core as core
import threading
import time
import queue

class BackgroundServicePlugin(core.IServicePlugin):
    def __init__(self):
        super().__init__()
        self._state = core.ServiceState.Stopped
        self._health = core.ServiceHealth.Unknown
        self._priority = core.ServicePriority.Normal
        self._execution_mode = core.ServiceExecutionMode.WorkerThread
        self._worker_thread = None
        self._stop_event = threading.Event()
        self._task_queue = queue.Queue()

    def name(self) -> str:
        return "BackgroundServicePlugin"

    def version(self) -> str:
        return "1.0.0"

    def description(self) -> str:
        return "Background service plugin with worker thread"

    # Service lifecycle methods
    def start_service(self):
        if self._state != core.ServiceState.Stopped:
            return

        self._state = core.ServiceState.Starting
        self._stop_event.clear()

        # Start worker thread
        self._worker_thread = threading.Thread(target=self._worker_loop)
        self._worker_thread.daemon = True
        self._worker_thread.start()

        self._state = core.ServiceState.Running
        self._health = core.ServiceHealth.Healthy
        print(f"Service {self.name()} started successfully")

    def stop_service(self):
        if self._state != core.ServiceState.Running:
            return

        self._state = core.ServiceState.Stopping
        self._stop_event.set()

        # Wait for worker thread to finish
        if self._worker_thread and self._worker_thread.is_alive():
            self._worker_thread.join(timeout=5.0)

        self._state = core.ServiceState.Stopped
        self._health = core.ServiceHealth.Unknown
        print(f"Service {self.name()} stopped")

    def service_state(self) -> core.ServiceState:
        return self._state

    def service_health(self) -> core.ServiceHealth:
        return self._health

    def _worker_loop(self):
        while not self._stop_event.is_set():
            try:
                # Process tasks from queue
                if not self._task_queue.empty():
                    task = self._task_queue.get(timeout=1.0)
                    print(f"Processing task: {task}")
                    time.sleep(0.5)  # Simulate work
                else:
                    time.sleep(0.1)
            except queue.Empty:
                continue
            except Exception as e:
                print(f"Error in worker loop: {e}")
                self._health = core.ServiceHealth.Warning

# Usage example
service = BackgroundServicePlugin()
service.start_service()
# ... use service
service.stop_service()
```

### Transaction Management Example

```python
# transaction_plugin.py
import qtforge.transactions as tx

class TransactionalPlugin:
    def __init__(self):
        self._transaction_manager = tx.get_transaction_manager()
        self._data_store = {}

    def perform_atomic_operation(self, operations: list):
        """Perform multiple operations atomically."""
        # Begin transaction
        tx_id = self._transaction_manager.begin_transaction(tx.IsolationLevel.ReadCommitted)

        try:
            # Perform all operations
            for op in operations:
                if op['type'] == 'create':
                    self._data_store[op['key']] = op['value']
                elif op['type'] == 'update':
                    if op['key'] in self._data_store:
                        self._data_store[op['key']] = op['value']
                    else:
                        raise KeyError(f"Key {op['key']} not found")

            # Commit transaction
            result = self._transaction_manager.commit_transaction(tx_id)
            return result is not None

        except Exception as e:
            # Rollback on error
            self._transaction_manager.rollback_transaction(tx_id)
            print(f"Transaction rolled back due to error: {e}")
            return False

# Usage example
plugin = TransactionalPlugin()
operations = [
    {'type': 'create', 'key': 'user1', 'value': {'name': 'Alice', 'age': 30}},
    {'type': 'update', 'key': 'user1', 'value': {'name': 'Alice Smith', 'age': 31}}
]
success = plugin.perform_atomic_operation(operations)
```

## Key Features Demonstrated

1. **Scientific Computing**: NumPy, Pandas, SciPy integration
2. **Machine Learning**: Scikit-learn model training and prediction
3. **Data Visualization**: Matplotlib and Seaborn plotting
4. **Web Scraping**: BeautifulSoup and requests for data collection
5. **Async Operations**: Asynchronous data processing
6. **Error Handling**: Comprehensive error handling and logging
7. **Message Integration**: Full integration with QtForge message bus
8. **Service Plugins**: Background service lifecycle management (v3.2.0+)
9. **Transaction Management**: Atomic operations with rollback support (v3.2.0+)
10. **Advanced Interfaces**: Enhanced plugin capabilities and composition (v3.2.0+)

## Best Practices

1. **Virtual Environments**: Use isolated Python environments
2. **Dependency Management**: Proper package version management
3. **Error Handling**: Graceful error handling and recovery
4. **Resource Management**: Proper cleanup of Python resources
5. **Performance**: Efficient data processing and memory usage
6. **Logging**: Comprehensive logging for debugging

## Next Steps

- **[Python Integration Tutorial](../tutorials/python-integration-tutorial.md)**: Detailed Python integration guide
- **[Advanced Examples](advanced.md)**: More complex Python patterns
- **[Performance Optimization](../user-guide/performance-optimization.md)**: Python performance tuning
- **[Deployment Guide](../deployment/python-deployment.md)**: Deploying Python plugins
