from threading import Thread

def spawn_thread(function, arguments=(), end_on_exit=False):
    target_thread = Thread(target=function, args=arguments)
    target_thread.start()
    if end_on_exit is True:
        target_thread.isDaemon()
    return target_thread