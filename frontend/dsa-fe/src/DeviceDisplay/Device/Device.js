import React from 'react';
import { useState,useEffect } from 'react';
import './Device.css';
import lightOn from '../../images/light-on.png';
import lightOff from '../../images/light-off.png';
import motionOff from '../../images/motion-off.png';
import motionOn from '../../images/motion-on.png';
import {Light,Motion} from '../../constants'

const Device = ({id,name,value,details,sensor,handleConnect}) =>{
    const [imgSrc, setImgSource] = useState();

    useEffect(()=>{
        switch(name){
            case "light":
                setImgSource(value === 1 ? lightOn : lightOff);
                break;
            case "motion":
                setImgSource(value === 1 ? motionOn : motionOff);
                break;
        }
    })

    const getLabel = () => {
        switch(name){
            case "light":
                return value ? Light.on : Light.off;
            case "motion":
                return value ? Motion.detected : Motion.notDetected;
        }
    }

    const renderButton = () => {
        if(sensor){
            return <button onClick={()=>{handleConnect(id)}} className="ConnectButton">Connect</button>
        }
    }

    return(
        <div className="DeviceContainer">
            <div>
                <img className="Image" src={imgSrc}></img>
            </div>
            <div className="Label">
            <label>{getLabel()}</label>
            </div>
            <div>
                <label>Details: {details}</label>
            </div>
            {renderButton()}
        </div>
    )
}

export default Device