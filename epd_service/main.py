from uplink import Uplink
# from downlink import dl
# from task import task
import sys
import os
from epd_log import write_log_file

if __name__ == "__main__":
    sys.path.append(os.path.dirname(os.path.abspath(__file__)))
    write_log_file('system','epd service start...')
    uplinkHandler = Uplink()
    # task.start_service()
    uplinkHandler.begin()
    uplinkHandler.end()
    # downlink.stop()
    