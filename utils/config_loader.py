import os
import yaml

DP_USER = os.environ["DPLUDO"]

    
def load_test_config():
    try:
        with open(DP_USER+"config/test_config.yml", "r") as f:
            config = yaml.load(f, Loader=yaml.FullLoader)
        return config

    except Exception as e: 
        raise e


def load_analysis_config():
    try:
        with open(DP_USER+"config/analysis_config.yml", "r") as f:
            config = yaml.load(f, Loader=yaml.FullLoader)
        return config

    except Exception as e: 
        raise e