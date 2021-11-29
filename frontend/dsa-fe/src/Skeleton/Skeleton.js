import React from 'react'
import {useState, useEffect} from 'react'
import './Skeleton.css'
import DeviceDisplay from '../DeviceDisplay/DeviceDisplay'
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import Dialog from '@material-ui/core/Dialog';
import axios from 'axios';



const Skeleton = () => {    
    const [devices,setDevices] = useState([]);
    const [openDialog,setOpenDialog] = useState(false);
    const [connectId, setConnectId] = useState(null);

    useEffect(() => {
       (async () => {
            await getData();
       })();
    },[])

    useEffect(()=>{
        let handle = setInterval(getData,2000);    
    
        return ()=>{
          clearInterval(handle);
        }
      });

    const getData= async () => {
    const data = await axios.get("http://localhost:8080/http://192.168.0.1/devices");
    if(data.data === null){
        setDevices([]);
        console.log("No devices found");
        return;
    }
    const finalObjects = [];
    const objects = data.data.split("#");
    objects.forEach(element => {
        const fields = element.split("-");
        const obj = {
        id:parseInt(fields[0]),
        type:fields[2],
        name:fields[3],
        details:fields[4],
        value:parseInt(fields[1])
        }
        finalObjects.push(obj);
    });
    finalObjects.pop();
        setDevices(finalObjects);
}

    const handleConnect = (id) => {
        setConnectId(id);
        setOpenDialog(true);
    }   

    const handleSelection = async (id) => {
        await axios.post("http://localhost:8080/http://192.168.0.1/connect",`${connectId}-${id}#`);
        setOpenDialog(false);
    }

    const handleRequest = async () => {
        await axios.post("http://localhost:8080/http://192.168.0.1/devices","0#1-0-output-door-Garage_Door#2-0-output-light-Garden_Light#3-0-output-light-Kitchen_Light#4-0-sensor-motion-Kitchen_Entry#5-0-sensor-ultrasonic-Garage_door#");
    }

    const handleChange = async () => {
        await axios.post("http://localhost:8080/http://192.168.0.1/devices","0#4-1-sensor-motion-Kitchen_Entry#5-1-sensor-ultrasonic-Garage_door#");
    }


    const renderDialog = () => {
        return(
            <Dialog open={openDialog} onClose={()=>{setOpenDialog(false)}}>
                <DialogTitle>
                    Select which output you would like to connect with
                </DialogTitle>
                <DialogContent>
                    <div className="SelectionContainer">
                    {devices.filter(dev => dev.type === "output").map((out,index) =>{
                        return(<div key={out.id} className="SelectionButton" onClick={() => handleSelection(out.id)}>{out.details}</div>)
                    })}
                    </div>
                </DialogContent>
            </Dialog>   
        )
    }

    return(
        <div className="GlobalContainer">
            <div>
                <div className="TitleContainer">
                    <h1 className="Title">Smart home system management</h1>
                </div>
                <hr/>
                { devices.length === 0 ? <h1>Waiting for devices...</h1> :
                (<div className="Content">
                <h1>Available Sensors</h1>
                <DeviceDisplay devices={devices.filter(dev => dev.type === "sensor")} sensors handleConnect={handleConnect}/>
                <h1>Available Outputs</h1>
                <DeviceDisplay devices={devices.filter(dev => dev.type === "output")}/>  
                </div>)
                }
            {renderDialog()}
            </div>   
            <button onClick={handleRequest}>Insert devices</button>
            <button onClick={handleChange}>Change values</button>
        </div>
    )

}

export default Skeleton;