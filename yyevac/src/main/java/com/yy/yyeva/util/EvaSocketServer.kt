package com.yy.yyeva.util

import android.net.LocalServerSocket
import android.net.LocalSocket

class EvaSocketServer {
    val TAG = "EvaSocket"
    val socketName = "eva_local_socket"
    var thread: Thread? = null
    var serverSocket: LocalServerSocket? = null

    fun startServer() {
        thread = Thread {
            try {
                serverSocket = LocalServerSocket(socketName)
                while (true) {
                    val clientSocket = serverSocket!!.accept()  //阻塞，知道有新的连接
                    handleClient(clientSocket)
                }
            } catch (e: Exception) {
                ELog.e(TAG, e.toString())
                closeServer()
            }
        }
        thread?.start()
    }

    fun handleClient(clientSocket: LocalSocket) {
        clientSocket.inputStream.bufferedReader().use {
            reader->
            //读取数据
            val message = reader.readLine()
            ELog.i(TAG, "message $message")

        }
    }

    fun closeServer() {
        serverSocket?.close()
        thread?.join()
    }
}