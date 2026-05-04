# Very simple server chatting system

This program is a very simple chat system demo made in C using the **enet** library. This program is not meant for professional use and is made for **educational purposes only**.

## Building

1. Clone this repository and open a terminal in the cloned repo
2. Run `make` command:
   ```bash
   make
   ```
3. This will drop 3 files into the `/build` directory:
   1. `server.exe` → the server
   2. `./client/sender.exe` → CLI for sending messages
   3. `./client/listener.exe` → a standalone client for receiving and displaying messages

## Usage

1. Start the server

   ```bash
   $ ./build/server.exe
   Server is running on port 2256!
   ```
2. Launch the listener

   ```bash
   $ ./build/client/listener.exe
   Enter valid IPv4 address: 127.0.0.1
   Connected to server 127.0.0.1 on port 2256.

   Listener (127.0.0.1) connected.
   ```
3. Open another terminal and run the sender:

   ```bash
   $ ./build/client/sender.exe
   Enter username: testuser1
   Enter a valid IPv4 address: 127.0.0.1
   Connection attemp no. 1
   Connected to server 127.0.0.1 on port 2256.
   Connected as 'testuser1'

   testuser1 >> 
   ```
4. Write some messages:

   ```bash
   testuser1 >> hello world!
   ```

   You should now see your messages in the listener:

   ```bash
   Client 'testuser1' (127.0.0.1) connected.
   testuser1 >> hello world!
   ```

You can chat with other people aswell, just make sure they connect to your server (your ip address). Both port forwarding and using a third-party VPN program (Radmin VPN, Hamachi, ...) work.

## Notes

* **This project was originally created in february 2026; I'm uploading it to GitHub just for archival purposes**
* **This program is made for educational purposes only.**
