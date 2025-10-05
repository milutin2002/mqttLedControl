package com.example.controlled

import android.annotation.SuppressLint
import android.util.Log
import com.hivemq.client.mqtt.MqttClient
import com.hivemq.client.mqtt.datatypes.MqttQos
import com.hivemq.client.mqtt.mqtt3.Mqtt3AsyncClient
import java.nio.charset.StandardCharsets

val topicSet=  "pico/led/set"
val topicStatus ="pico/led/status"

object MqttController {
    private var client: Mqtt3AsyncClient? =null

    fun createClient(brokerHost: String,brokerPort:Int = 1883,clientId: String="android-"+ System.currentTimeMillis()){
        if(client!=null){
            return
        }
        Log.i("Creating","Creating client")
        client= MqttClient.builder().useMqttVersion3().identifier(clientId).serverHost(brokerHost).serverPort(brokerPort).buildAsync()

    }

    fun connect(onConnected:()->Unit, onError:(Throwable)-> Unit){
        val c=client?: return onError(IllegalStateException("Client not created"))
        c.connect().whenComplete { _,t ->
            if(t!=null){
                onError(t)
            }
            else{
                onConnected()
            }
         }

    }
    @SuppressLint("CheckResult")
    fun subscribeStatus(onMessage:(String)->Unit,onError: (Throwable) -> Unit){
        val c=client?: return onError(IllegalStateException("Client not created"))
        c.subscribeWith().topicFilter("pico/led/status").qos(MqttQos.AT_LEAST_ONCE).callback {
                publish ->
                val buf=publish.payload.orElse(null)
                val payload = if(buf!=null){
                    val dup=buf.slice()
                    val bytes= ByteArray(dup.remaining())
                    dup.get(bytes)
                    String(bytes, StandardCharsets.UTF_8)
                }
            else ""
            onMessage(payload)

        }.send().
                whenComplete { _,t->
                    if(t!=null){
                        onError(t)
                    }
                 }
    }

    fun publishSet(payload: String,onError: (Throwable) -> Unit){
        val c=client?: return onError(IllegalStateException("Client not created"))
        c.publishWith().topic(topicSet).qos(MqttQos.AT_MOST_ONCE).payload(payload.toByteArray(
            StandardCharsets.UTF_8)).send().whenComplete { _,t->{
                if(t!=null){
                    onError(t)
                }
        } }
    }
    fun disconnect(){
        client?.disconnect()
        client=null
    }
}