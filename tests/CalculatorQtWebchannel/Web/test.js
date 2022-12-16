
function output(message)
{
    var output = document.getElementById("output");
    if(output != null)
    {
        output.innerHTML = output.innerHTML + message + "\n";
    }
}

function handleWebChannel(socket)
{
    new QWebChannel(socket, function(channel)
    {
        window.calculator = channel.objects.Calculator;
        setInterval(function() {
           // calculator.setMode(1);
            calculator.screen__setColor(calculator.EColor.Yellow);
            if(calculator.mode == calculator.EMode.Bright)
            {
                calculator.setMode(calculator.EMode.Dimmed)
            }
            else
            {
                calculator.setMode(calculator.EMode.Bright)
            }
        }, 1000);
        calculator.screen__colorChanged.connect(function(color){
            console.log("screen__colorChanged " + color)
            output("color " + color);
        });
        calculator.modeChanged.connect(function(){
            console.log("modeChanged " + calculator.mode)
            output("mode " + calculator.mode);
        });
    });
}

function connectToServer()
{
    if (location.search != "")
        var baseUrl = (/[?&]webChannelBaseUrl=([A-Za-z0-9\-:/\.]+)/.exec(location.search)[1]);
    else
        var baseUrl = "ws://localhost:12345";

    output("Connecting to WebSocket server at " + baseUrl + ".");
    var socket = new WebSocket(baseUrl);

    socket.onclose = function() 
    {
        output("web channel closed");
        output('Socket is closed. Reconnect will be attempted in 1 second.');
        setTimeout(function() {
            output("Timeout -- reconnecting")
            connectToServer();
        }, 100);
    };

    socket.onerror = function(error) 
    {
        output("web channel error: " + error);
        socket.close();
    };

    socket.onopen = function(){
        output("WebSocket connected, setting up QWebChannel.");
        handleWebChannel(socket);
    }
}


window.onload = function()
{
    var textarea = document.getElementById("output");
    textarea.scrollTop = textarea.scrollHeight;
    if(typeof qt !== 'undefined')
    {
        output("--1")
        handleWebChannel(qt.webChannelTransport)
    }
    else
    {
        output("--2")
        connectToServer();
    }
}