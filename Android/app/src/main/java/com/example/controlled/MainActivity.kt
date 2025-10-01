package com.example.controlled

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import com.example.controlled.ui.theme.ControlLedTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.eclipse.paho.android.service.MqttAndroidClient
import org.eclipse.paho.client.mqttv3.IMqttActionListener
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken
import org.eclipse.paho.client.mqttv3.IMqttToken
import org.eclipse.paho.client.mqttv3.MqttCallback
import org.eclipse.paho.client.mqttv3.MqttClient
import org.eclipse.paho.client.mqttv3.MqttConnectOptions
import org.eclipse.paho.client.mqttv3.MqttMessage

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            ControlLedTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    LedControllScreen()
                }
            }
        }
    }
}


@Composable
fun LedControllScreen(){
    val context= LocalContext.current
    val scope= rememberCoroutineScope()

    val broker =remember {   "tcp://broker.emqx.io:1883"}
    var statusText by remember { mutableStateOf("Disconnected") }

    var lastLedStatus by remember{ mutableStateOf<String?>(null) }


    var client by remember { mutableStateOf<MqttAndroidClient?>(null) };
    val topicSet= remember { "pico/led/set" }
    val topicStatus =remember { "pico/led/status" }
    fun connect(){
        val cId="android-"+ System.currentTimeMillis()
        val c= MqttAndroidClient(context,broker,cId)
        client=c
        c.setCallback(object : MqttCallback{
            override fun connectionLost(cause: Throwable?) {
                statusText="Disconnected"
            }

            override fun messageArrived(
                topic: String?,
                message: MqttMessage?
            ) {
                if(topic==topicStatus && message!=null){
                    val msg=message.toString()
                    lastLedStatus=msg
                    statusText="Led status: $msg"
                }
            }

            override fun deliveryComplete(token: IMqttDeliveryToken?) {
                TODO("Not yet implemented")
            }

        })
        val opts= MqttConnectOptions().apply {
            isAutomaticReconnect=true
            isCleanSession=true
        }
        statusText="Connecting"
        c.connect(opts,null,object : IMqttActionListener{
            override fun onSuccess(asyncActionToken: IMqttToken?) {
                statusText="Connected"
                c.subscribe(topicStatus,0,null,object : IMqttActionListener{
                    override fun onSuccess(asyncActionToken: IMqttToken?) {
                        statusText="Ready to listen"
                    }

                    override fun onFailure(
                        asyncActionToken: IMqttToken?,
                        exception: Throwable?
                    ) {
                        statusText="Subscribe failed"
                    }
                })
            }

            override fun onFailure(
                asyncActionToken: IMqttToken?,
                exception: Throwable?
            ) {
                statusText="Failed to connect"
            }
        })
    }
    fun publish(payload: String){
        val c=client?: run {
            statusText="Not connected"
            return
        }
        if(!c.isConnected){
            statusText="Not connected"
            return
        }
        scope.launch(Dispatchers.IO) {
            try{
                val msg = MqttMessage(payload.toByteArray()).apply {
                    qos=0
                    isRetained=false
                }
                c.publish(topicSet,msg)
                statusText="Sending payload"
            }catch (e: Exception){
                statusText="Sending error: ${e.message}"
            }
        }
    }
    connect()
    Column (Modifier.padding(20.dp)){
        Text(text = "IOT LED (Mqtt)", style = MaterialTheme.typography.titleLarge)
        Spacer(Modifier.padding(12.dp))
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)){
            Button(onClick = {publish("ON")}, modifier = Modifier.weight(1f)) {
                Text("Turn LED on")
            }
            Button(onClick = {publish("OFF")}, modifier = Modifier.weight(1f)) {
                Text("Turn LED off")
            }

            Spacer(Modifier.height(12.dp))
            lastLedStatus?.let { Text(text = it) }
        }
    }
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    ControlLedTheme {
        Greeting("Android")
    }
}