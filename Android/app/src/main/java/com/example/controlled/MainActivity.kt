package com.example.controlled

import android.os.Bundle
import android.util.Log
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
import androidx.compose.runtime.LaunchedEffect
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
import io.reactivex.plugins.RxJavaPlugins
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch


class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        RxJavaPlugins.setErrorHandler { e ->
            android.util.Log.e("RxJava", "Unhandled error", e)
        }
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

    val broker =  "broker.emqx.io"
    var statusText by remember { mutableStateOf("Disconnected") }

    var lastLedStatus by remember{ mutableStateOf<String?>(null) }


    LaunchedEffect(Unit) {
        MqttController.createClient(broker, brokerPort = 8883)
        MqttController.connect(onConnected = {
            try {
                Log.i("Connection", "Established")
                statusText = "Connected"
            }catch (e:Exception){
                Log.e("MQTT", "onMessage error", e)
            }
            Log.i("ConnectInfo","Reached connection stage")
            MqttController.subscribeStatus(onMessage = {msg->
                lastLedStatus = "Led status $msg"

            }, onError = {
                e->statusText="Subscribe error $e"
            })
        }, onError = {e->{
            statusText="Connection failed: $e"
        }})
    }
    Column (Modifier.padding(20.dp)){
        Text(text = statusText, style = MaterialTheme.typography.titleLarge)
        Spacer(Modifier.padding(12.dp))
        Text(text = "IOT LED (Mqtt)", style = MaterialTheme.typography.titleLarge)
        Spacer(Modifier.padding(12.dp))
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)){
            Button(onClick = { MqttController.publishSet("ON", onError = {e->
                statusText="Publish error $e"
            })}, modifier = Modifier.weight(1f)) {
                Text("Turn LED on")
            }
            Button(onClick = {MqttController.publishSet("OFF", onError = {e->
                statusText="Publish error $e"
            })}, modifier = Modifier.weight(1f)) {
                Text("Turn LED off")
            }
        }
        Spacer(Modifier.height(12.dp))
        lastLedStatus?.let { Text(text = it) }
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