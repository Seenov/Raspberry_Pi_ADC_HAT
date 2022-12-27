from queue import Queue


class MQueues:
    """ Should be used as it is """

    @staticmethod
    def create_queue(capacity_of_queue=0, queue_id="", show_output=True):
        """ It creates a queue with a max capacity of elements """
        if capacity_of_queue <= 0:
            capacity_of_queue = 0
            if show_output is True:
                print(f"Total capacity of this queue {queue_id} is :: unlimited")
        else:
            if show_output is True:
                print(f"Total capacity of this queue {queue_id} is :: {capacity_of_queue}")
        return Queue(maxsize=capacity_of_queue)

    @staticmethod
    def add_to_queue(spawned_queue, target_element=None, show_output=False):
        """ It adds something to the Queue provided in the function """
        if spawned_queue.full() is True:
            if show_output is True:
                print(f"Queue Already Full :: MaxSize {spawned_queue.qsize()}")
            return None
        else:
            if target_element is None:
                return None
            if target_element is not None:
                try:
                    if show_output is True:
                        print(str(target_element) + " ::" + " Adding to the Queue")
                    spawned_queue.put(target_element)
                    return f"Successfully Added {target_element}"
                except Exception:
                    if show_output is True:
                        print(str(target_element) + " ::" + " Failed adding to the Queue")
                    return None

    @staticmethod
    def get_first_from_queue(spawned_queue, force_no_wait=False, show_output=False):
        """ Get something specific from the Queue or sequentially using FIFO method """
        try:
            if force_no_wait is True:
                value_from_queue = spawned_queue.get_nowait()
            else:
                value_from_queue = spawned_queue.get()
            if show_output is True:
                print(f"{value_from_queue} is present in the queue")
            return value_from_queue
        except Exception:
            if show_output is True:
                print(f"Nothing in the queue")
            return None