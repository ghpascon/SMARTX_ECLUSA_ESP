import socket
import select
import threading
import time
import sys

HOST = "192.168.1.140"
PORT = 23
SEND_INTERVAL = 2.0  # segundos
RECV_TIMEOUT = 1.0    # segundos por tentativa de leitura

running = True

def read_line(sock, timeout=RECV_TIMEOUT):
    """
    Lê do socket até encontrar '\n' (ignora '\r') ou até timeout.
    Retornos:
      - bytes: linha lida (sem '\n'), possivelmente parcial se timeout
      - b'': conexão fechada
      - None: nenhum dado dentro do timeout
    """
    end_time = time.time() + timeout
    buf = bytearray()

    while time.time() < end_time:
        remaining = max(0.0, end_time - time.time())
        r, _, _ = select.select([sock], [], [], remaining)
        if not r:
            break  # timeout sem dados

        chunk = sock.recv(1)
        if not chunk:
            # conexão fechada
            if buf:
                return bytes(buf)
            return b''

        c = chunk[0]
        if c == 10:  # '\n'
            return bytes(buf)
        if c == 13:  # '\r' -> ignora
            continue
        buf.append(c)

    if buf:
        return bytes(buf)
    return None

def receiver(sock):
    global running
    try:
        while running:
            line = read_line(sock, timeout=RECV_TIMEOUT)
            if line is None:
                continue  # nada recebido no intervalo
            if line == b'':
                print("Conexão encerrada pelo servidor.")
                running = False
                break
            print("RX:", line.decode("utf-8", errors="replace"))
    except Exception as e:
        print("Receiver erro:", e)
        running = False

def main():
    global running
    try:
        print(f"Conectando em {HOST}:{PORT} ...")
        sock = socket.create_connection((HOST, PORT), timeout=5)
        sock.setblocking(False)
        print("Conectado.")

        t = threading.Thread(target=receiver, args=(sock,), daemon=True)
        t.start()

        while running:
            msg = "hello\n"
            # try:
            #     sock.sendall(msg.encode("utf-8"))
            #     print(f"TX: {msg}")
            # except Exception as e:
            #     print("Erro ao enviar:", e)
            #     running = False
            #     break
            time.sleep(SEND_INTERVAL)

        sock.close()
    except KeyboardInterrupt:
        running = False
        print("\nEncerrando por Ctrl+C...")
    except Exception as e:
        print("Erro de conexão:", e)
        running = False

if __name__ == "__main__":
    main()
