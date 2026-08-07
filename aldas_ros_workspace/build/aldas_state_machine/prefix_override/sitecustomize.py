import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/zangetsu/native_projects/Project-ALDAS/aldas_ros_workspace/install/aldas_state_machine'
