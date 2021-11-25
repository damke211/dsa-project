import React from 'react'
import {useState} from 'react'
import './Skeleton.css'
import DeviceDisplay from '../DeviceDisplay/DeviceDisplay'
import DialogContent from '@material-ui/core/DialogContent';
import DialogTitle from '@material-ui/core/DialogTitle';
import DialogContentText from '@material-ui/core/DialogContentText';
import Dialog from '@material-ui/core/Dialog';
import axios from 'axios';


const mockDevices = [
    {
        id:1,
        type:"sensor",
        state:1,
        name:"motion",
        details:"Main door"
    },
    {
        id:3,
        type:"output",
        state:1,
        name:"light",
        details:"Main door"
    },
    {
        id:4,
        type:"output",
        state:0,
        name:"light",
        details:"Garden"
    },
    {
        id:5,
        type:"output",
        state:1,
        name:"light",
        details:"Main door"
    }, 
    {
        id:6,
        type:"output",
        state:0,
        name:"light",
        details:"Garden"
    },
    {
        id:1,
        type:"sensor",
        state:1,
        name:"ultrasonic",
        details:"Garage door"
    },
    {
        id:1,
        type:"sensor",
        state:1,
        name:"ldr",
        details:"Roof"
    },

]

const Skeleton = () => {
    const sensors = mockDevices.filter(dev => dev.type === "sensor");
    const outputs = mockDevices.filter(dev => dev.type === "output");
    const [openDialog,setOpenDialog] = useState(false);
    const [connectId, setConnectId] = useState(null);

    const handleConnect = (id) => {
        setConnectId(id);
        setOpenDialog(true);
    }   

    const handleSelection = (id) => {
        alert("Connecting " + connectId + " with " + id);
        setOpenDialog(false);
        // poslati http request da odradi ovo spajanje
    }

    const handleRequest = async () => {
        const data = await axios.get("http://localhost:8080/http://192.168.0.1/off");
        console.log(data.data);
    }


    const renderDialog = () => {
        return(
            <Dialog open={openDialog} onClose={()=>{setOpenDialog(false)}}>
                <DialogTitle>
                    Select which output you would like to connect with
                </DialogTitle>
                <DialogContent>
                    <div className="SelectionContainer">
                    {outputs.map(out =>{
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
                    <DeviceDisplay devices={sensors} sensors handleConnect={handleConnect}/>
                    <h1>Available Outputs</h1>
                    <DeviceDisplay devices={outputs}/>  
                </div>
                {renderDialog()}
            </div>   
            <button onClick={handleRequest}>Request</button>
        </div>
    )

}

export default Skeleton;