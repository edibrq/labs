import threading
import time
import random

class PhoneNetwork:
    def __init__(self, participants):
        self.participants = participants
        self.confirmation = False
        self.confirmation_semaphore = threading.Semaphore(1)
        self.phone_semaphores = {p: threading.Semaphore(1) for p in participants}
        self.confirmation_source = {}

    def call(self, caller, callee):
        # Caller tries to acquire their own phone
        self.phone_semaphores[caller].acquire()
        try:
            # Try to acquire callee's phone without blocking
            if self.phone_semaphores[callee].acquire(blocking=False):
                try:
                    print(f"{caller} звонит {callee}.")
                    time.sleep(random.uniform(0.1, 0.3))  # Simulate conversation
                    return True
                finally:
                    # Release callee's phone after the call
                    self.phone_semaphores[callee].release()
            else:
                return False
        finally:
            # Release caller's phone
            self.phone_semaphores[caller].release()

    def poluekt_activity(self):
        time.sleep(random.uniform(0.1, 0.5))
        print("Полуэкт пытается позвонить...")

        for person in self.participants[1:]:
            success = self.call('Poluekt', person)
            if success:
                # Acquire semaphore to safely update shared confirmation data
                self.confirmation_semaphore.acquire()
                try:
                    self.confirmation = True
                    self.confirmation_source[person] = 'Poluekt'
                finally:
                    self.confirmation_semaphore.release()
                print(f"Полуэкт сообщил {person}, что он в порядке.")
                break
        else:
            print("Полуэкту не удалось никому дозвониться.")

    def member_activity(self, name):
        others = [p for p in self.participants if p != name and p != 'Poluekt']

        while True:
            # Check if confirmation has been received
            self.confirmation_semaphore.acquire()
            try:
                if self.confirmation and name in self.confirmation_source:
                    print(f"{name} получил(а) подтверждение от {self.confirmation_source[name]} и прекращает звонки.")
                    break
            finally:
                self.confirmation_semaphore.release()

            # Try calling other participants
            random.shuffle(others)
            for other in others:
                success = self.call(name, other)
                if success:
                    print(f"{name} разговаривает с {other}.")
                    # Check if the other person knows about Poluekt
                    self.confirmation_semaphore.acquire()
                    try:
                        if self.confirmation:
                            if other in self.confirmation_source:
                                self.confirmation_source[name] = other
                                print(f"{name} узнал(а), что Полуэкт в порядке от {other}.")
                                return
                    finally:
                        self.confirmation_semaphore.release()
            time.sleep(random.uniform(0.1, 0.2))

# Main part of the program
participants = ['Poluekt', 'Grandma1', 'Grandma2', 'Mother', 'Girlfriend1', 'Girlfriend2']

network = PhoneNetwork(participants)
threads = []

# Thread for Poluekt
t_poluekt = threading.Thread(target=network.poluekt_activity)
threads.append(t_poluekt)

# Threads for other participants
for person in participants[1:]:
    t = threading.Thread(target=network.member_activity, args=(person,))
    threads.append(t)

# Start all threads
for t in threads:
    t.start()

# Wait for all threads to complete
for t in threads:
    t.join()

print("Все участники получили подтверждение о Полуэкте.")