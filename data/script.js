%CONSTS%;

let hasChanged = Array(SETTINGS_SIZE).fill(false);

window.onload = function init() {
    for(let i=0; i<SETTINGS_SIZE; i++) {
        if(configurable[i] == 1) {
            document.getElementById(i).addEventListener("change", () => {hasChanged[i] = true;});
        }
    }
    for(let i=0; i<SETTINGS_SIZE; i++) {
        if(configurable[i] == 1) {
            document.getElementById(i).addEventListener("change", () => {send();});
        }
    }
}


let source;

function setEventSource() {
    if (!!window.EventSource) {
        source = new EventSource('/events');

        source.addEventListener('open', function(e) {
            console.log("Events Connected");
        }, false);

        source.addEventListener('error', function(e) {
            if (e.target.readyState != EventSource.OPEN) {
                console.log("Events Disconnected");
            }
        }, false);

        source.addEventListener('data', function(e) {
            console.log("data", e.data);
            const json = JSON.parse(e.data);
            for(let i=0; i<7; i++) {
                if(json.hasOwnProperty(i)) {
                    document.getElementById("voltage_" + i).innerText = "Voltage: " + json[i][0] + "V";
                    document.getElementById("resistance_" + i).innerText = "Resistance: " + json[i][1] + "Ω";
                    document.getElementById("value_" + i).innerText = "Value: " + json[i][2];
                } else {
                    document.getElementById("voltage_" + i).innerText = "Voltage: 0V";
                    document.getElementById("resistance_" + i).innerText = "Resistance: 0Ω;";
                    document.getElementById("value_" + i).innerText = "Value: 0";
                }
            }
        }, false);

        source.addEventListener('frametime', function(e) {
            document.getElementById("frametime").innerText = "frametime: " + e.data;
        }, false);
    }
}

setEventSource();

let responseTime = 0;

function send() {
    source.close();
    let data = new FormData();

    for(let i=0; i<SETTINGS_SIZE; i++) {
        if(hasChanged[i] && configurable[i]) {
            let type = document.getElementById(i).getAttribute("type");
            switch (type) {
                case "checkbox" : data.append(i, document.getElementById(i).checked ? 1 : 0);break;
                default : data.append(i, document.getElementById(i).value);
            }
        }
    }

    let xhr = new XMLHttpRequest();
    xhr.open('POST', 'settingsSet');
    xhr.onload = function() {
        if(this.response == "success") {
            hasChanged.fill(false);
            document.getElementById("response").style.color = "green";
            document.getElementById("response").innerHTML = "Saved successfully";
        } else {
            document.getElementById("response").style.color = "red";
            document.getElementById("response").innerHTML = "Saving failed: " + this.status + " " + this.response;
        }
        console.log(this.status);
        responseTime = Date.now();
        setTimeout(function() {
        if(Date.now() - responseTime >= 2000)
            document.getElementById("response").innerHTML = "";
        }, 2000);
    };
    xhr.send(data);
    setEventSource();
    return false;
}

function post(s, reload = false) {
    source.close();
    let xhr = new XMLHttpRequest();
    xhr.open('POST', s);
    if(reload)
        xhr.onload = function(){
        console.log(this.status);
        window.location.reload(true);
    };
    xhr.send(null);
    setEventSource();
}

function calibration() {
    let data = new FormData();
    data.append("mac", MAC);
    let xhr = new XMLHttpRequest();
    xhr.open('POST', 'http://217.11.128.153:1337/calibration.php');
    xhr.onload = function() {
        console.log(this.status);
        console.log(this.response);
        const json = JSON.parse(this.response);
        for(let i=0; i<7; i++) {
            if(json.hasOwnProperty(i)) {
                let offset = INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE*i;
                hasChanged.fill(true, offset, offset + 2);
                document.getElementById(offset + 0).value = json[i][0];
                // document.getElementById(offset + 1).value = json[i][1];
                // document.getElementById(offset + 2).value = json[i][2];
            }
        }
        send();
            // window.location.reload(true);
    };
    xhr.send(data);
}

function screenshot() {
    source.close();
    let xhr = new XMLHttpRequest();
    xhr.open('POST', "screenshot");
    xhr.responseType = "arraybuffer";

    xhr.onload = function(){
        console.log(this.status)
        let arrayBuffer = this.response;
        console.log(arrayBuffer)
        if (arrayBuffer) {
            let RGBarray = new Uint8ClampedArray(arrayBuffer);
            let RGBAlength = RGBarray.length / 3 * 4;
            let RGBAarray = new Uint8ClampedArray(RGBAlength);

            for(let i=0; i<RGBAlength/4; i++) {
                RGBAarray[i*4 + 0] = RGBarray[i*3 + 0];
                RGBAarray[i*4 + 1] = RGBarray[i*3 + 1];
                RGBAarray[i*4 + 2] = RGBarray[i*3 + 2];
                RGBAarray[i*4 + 3] = 255;
            }

            console.log(RGBarray);
            console.log(RGBAarray);

            var canvas = document.getElementById('canvas');
            var ctx = canvas.getContext('2d');

            ctx.putImageData(new ImageData(RGBAarray, 480), 0, 0);

            var MIME_TYPE = "image/png";
            var imgURL = canvas.toDataURL(MIME_TYPE);
            var dlLink = document.getElementById('canvas_a');
            var now = new Date();

            dlLink.download = "esp_ss_" + now.getFullYear() + (now.getMonth()+1) + now.getDate() + now.getHours() + now.getMinutes() + now.getSeconds();
            dlLink.href = imgURL;
            dlLink.dataset.downloadurl = [MIME_TYPE, dlLink.download, dlLink.href].join(':');
            dlLink.style.display = "inline";

            setEventSource();
        }
    };
    xhr.send(null);
}

function preset(i) {
    let preset = parseInt(document.getElementById('input_' + i + '_preset').value);

    let input = INPUT_BEGIN_BEGIN + i * INPUT_SETTINGS_SIZE;
    let data = DATA_BEGIN_BEGIN + i * DATA_SETTINGS_SIZE;

    let presets = Array(3);
    presets[0] = [0, 0, 0, "v",                                                         false, "", "u", 0 ,0 ,0]; //emptyy
    presets[1] = [null, null, null, "max(0,(r-3)/(160-3)*10)",                          true, "Oil press", "bar", 0 ,12 ,1]; //oil pressure
    presets[2] = [null, null, null, "3800*298.15/(3800+(298.15)*log(r/58000))-273.15",  true, "Oil temp", "°C", 30 ,150 ,1]; //oil pressure

    for(let i=0; i<INPUT_SETTINGS_SIZE + DATA_SETTINGS_SIZE; i++) {
        if(presets[preset][i] != null) {
            if(i<INPUT_SETTINGS_SIZE)
                document.getElementById(input + i).value = presets[preset][i];
            else if(i==INPUT_SETTINGS_SIZE)
                document.getElementById(data + i - INPUT_SETTINGS_SIZE).checked = presets[preset][i];
            else
                document.getElementById(data + i - INPUT_SETTINGS_SIZE).value = presets[preset][i];
        }
    }

    hasChanged.fill(true, input, input + INPUT_SETTINGS_SIZE);
    hasChanged.fill(true, data, data + DATA_SETTINGS_SIZE);
    send();
}