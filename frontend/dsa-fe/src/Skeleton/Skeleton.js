import React from 'react'
import {useState, useEffect} from 'react'
import './Skeleton.css'
import DeviceDisplay from '../DeviceDisplay/DeviceDisplay'
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import DialogContentText from '@material-ui/core/DialogContentText';
import Dialog from '@material-ui/core/Dialog';
import axios from 'axios';



const Skeleton = () => {    
    const [devices,setDevices] = useState([]);
    const [openDialog,setOpenDialog] = useState(false);
    const [connectId, setConnectId] = useState(null);

    useEffect(async () => {
        // const data = await axios.get("http://localhost:8080/http://192.168.0.1/devices");
        // if(data.data === null)
        // {
        //     console.log("usla je u fail")
        //     setDevices([]);
        //     return;
        // }
        // const finalObjects = [];
        // const objects = data.data.split("#");
        // objects.pop()
        // objects.forEach(element => {
        //   const fields = element.split("-");
        //   const obj = {
        //     id:parseInt(fields[0]),
        //     type:fields[2],
        //     name:fields[3],
        //     details:fields[4].split('_').join(" "),
        //     value:parseFloat(fields[1])
        //   }
        //   finalObjects.push(obj);
        // });

        // setDevices(finalObjects);
    },[])

    useEffect(()=>{
        let handle = setInterval(getData,5000);    
    
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
    console.log(data.data);
    const finalObjects = [];
    const objects = data.data.split("#");
    objects.forEach(element => {
        const fields = element.split("-");
        const obj = {
        id:parseInt(fields[0]),
        type:fields[2],
        name:fields[3],
        details:fields[4],
        value:parseFloat(fields[1])
        }
        finalObjects.push(obj);
    });
    finalObjects.pop();
        setDevices(finalObjects);
        console.log(devices)   
}
    

    const handleConnect = (id) => {
        setConnectId(id);
        setOpenDialog(true);
    }   

    const handleSelection = async (id) => {
        const response = await axios.post("http://localhost:8080/http://192.168.0.1/connect","1-2");
        setOpenDialog(false);
        // poslati http request da odradi ovo spajanje
    }

    const handleRequest = async () => {
        const data = await axios.post("http://localhost:8080/http://192.168.0.1/devices","0#1-1-output-door-Garage_Door#2-0-output-light-Garden_Light#3-1-output-light-Kitchen_Light#");
        console.log(data);
    }

    const handleChange = async () => {
        const data = await axios.post("http://localhost:8080/http://192.168.0.1/devices","0#1-0-output-door-Garage_Door#2-1-output-light-Garden_Light#3-0-output-light-Kitchen_Light#");
        console.log(data);
    }


    const renderDialog = () => {
        return(
            <Dialog open={openDialog} onClose={()=>{setOpenDialog(false)}}>
                <DialogTitle>
                    Select which output you would like to connect with
                </DialogTitle>
                <DialogContent>
                    <div className="SelectionContainer">
                    {devices.filter(dev => dev.type === "output").map(out =>{
                        return(<div className="SelectionButton" onClick={() => handleSelection(out.id)}>{out.details}</div>)
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
                <div className="Content">
                    <h1>Available Sensors</h1>
                    <DeviceDisplay devices={devices.filter(dev => dev.type === "sensor")} sensors handleConnect={handleConnect}/>
                    <h1>Available Outputs</h1>
                    <DeviceDisplay devices={devices.filter(dev => dev.type === "output")}/>  
                </div>
                {renderDialog()}
            </div>   
            <button onClick={handleRequest}>Insert devices</button>
            <button onClick={handleChange}>Change values</button>
        </div>
    )

}

export default Skeleton;