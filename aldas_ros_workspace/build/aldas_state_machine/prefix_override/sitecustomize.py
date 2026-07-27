import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/shashi/Desktop/Git/Project-ALDAS/aldas_ros_workspace/install/aldas_state_machine'
