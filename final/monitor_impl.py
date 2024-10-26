import threading
import time
import random

class PhoneNetwork:
    def __init__(self, participants):
        self.participants = participants
        self.confirmation = False
        self.confirmation_lock = threading.Lock()
        self.phone_locks = {p: threading.Lock() for p in participants}
        self.confirmation_source = {}

    def call(self, caller, callee):
        acquired_caller = self.phone_locks[caller].acquire(blocking=False)
        acquired_callee = self.phone_locks[callee].acquire(blocking=False)

        if acquired_caller and acquired_callee:
            try:
                print(f"{caller} звонит {callee}.")
                time.sleep(random.uniform(0.1, 0.3))
                return True
            finally:
                self.phone_locks[callee].release()
                self.phone_locks[caller].release()
        else:
            if acquired_caller:
                self.phone_locks[caller].release()
            if acquired_callee:
                self.phone_locks[callee].release()
            return False

    def poluekt_activity(self):
        time.sleep(random.uniform(0.1, 0.5))
        print("Полуэкт пытается позвонить...")

        while True:
            for person in self.participants[1:]:
                success = self.call('Poluekt', person)
                if success:
                    with self.confirmation_lock:
                        self.confirmation = True
                        self.confirmation_source[person] = 'Poluekt'
                    print(f"Полуэкт сообщил {person}, что он в порядке.")
                    return
            time.sleep(random.uniform(0.1, 0.5))
            print("Полуэкт пытается снова позвонить...")

    def member_activity(self, name):
        others = [p for p in self.participants if p != name and p != 'Poluekt']

        while True:
            with self.confirmation_lock:
                if self.confirmation and name in self.confirmation_source:
                    print(f"{name} получил(а) подтверждение от {self.confirmation_source[name]} и прекращает звонки.")
                    break

            random.shuffle(others)
            for other in others:
                success = self.call(name, other)
                if success:
                    print(f"{name} разговаривает с {other}.")
                    with self.confirmation_lock:
                        if self.confirmation:
                            if other in self.confirmation_source:
                                self.confirmation_source[name] = other
                                print(f"{name} узнал(а), что Полуэкт в порядке от {other}.")
                                return
                    print(f"{name} закончила разговор с {other}.")
            time.sleep(random.uniform(0.1, 0.2))

# Main part of the program
num_of_girlfriends = int(input("Введите количество подруг: "))
participants = ['Poluekt', 'Grandma1', 'Grandma2', 'Mother'] + [f"Girlfriend{i}" for i in range(1, num_of_girlfriends + 1)]

network = PhoneNetwork(participants)
threads = []

t_poluekt = threading.Thread(target=network.poluekt_activity)
threads.append(t_poluekt)

for person in participants[1:]:
    t = threading.Thread(target=network.member_activity, args=(person,))
    threads.append(t)

for t in threads:
    t.start()

for t in threads:
    t.join()

print("Все участники получили подтверждение о Полуэкте.")