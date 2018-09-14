import os,sys
import update
import time
import multiprocessing

debug = False
def dprint(msg):
	global debug
	if debug:
		print(msg)

def load_extra_library():
	pass

def initAll(test = True):
	load_extra_library()
	if is_windows():
		multiprocessing.freeze_support()

	try:
		import start
		main_proc = start.Main()
		main_proc.start()

		update_now = False

		next_update_time = int(time.time()) + 5
		while True:
			if not main_proc.is_alive():
				dprint("[Init Info] marsnake terminated")
				break

			if not update_now and int(time.time()) > next_update_time:
				try:
					ret = update.check_and_update(main_proc)
					if ret:
						update_now = True
						continue

					next_update_time = int(time.time()) + 60*60 # wait for 1 hour
				except Exception as e:
					dprint("[Init Error]", str(e))

			time.sleep(1)

		# update_now will force kill us.
		# if you need to do other things,
		# Do it before this function !!!
		if update_now:
			update.update_now()
	except Exception as e:
		dprint('[Init Info] start marsnake failed with ' + str(e))

	return 0

if __name__ == '__main__':
	initAll()