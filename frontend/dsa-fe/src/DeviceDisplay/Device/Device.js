import React from 'react';
import { useState,useEffect } from 'react';
import './Device.css';
import lightOn from '../../images/light-on.png';
import lightOff from '../../images/light-off.png';
import motionOff from '../../images/motion-off.png';
import motionOn from '../../images/motion-on.png';
import ldrOn from '../../images/ldr-on.png';
import ldrOff from '../../images/ldr-off.png';
import doorOpen from '../../images/door-open.png';
import doorClosed from '../../images/door-close.png';
import usClose from '../../images/us-close.png';
import usFar from '../../images/us-far.png';
import alarmOn from '../../images/alarm-on.png';
import alarmOff from '../../images/alarm-off.png';


import {Labels} from '../../constants'

const Device = ({id,name,value,details,sensor,handleConnect}) =>{
    const [imgSrc, setImgSource] = useState();

    useEffect(()=>{
        switch(name){
            case "light":
                setImgSource(value === 1 ? lightOn : lightOff);
                break;
            case "motion":
                setImgSource(value === 0 ? motionOn : motionOff);
                break;
            case "ultrasonic":
                setImgSource(value === 0 ? usClose : usFar);
                break;
            case "alarm":
                setImgSource(value === 1 ? alarmOn : alarmOff);
                break;
            case "door":
                setImgSource(value === 1 ? doorOpen : doorClosed);
                break;
            case "ldr":
                setImgSource(value === 1 ? ldrOn : ldrOff);
                break;
            default:
                setImgSource();
        }
    },[name,value]);

    const getLabel = () => {
        switch(name){
            case "light":
                return value  ? Labels.light.on : Labels.light.off;
            case "motion":
                return value ? Labels.motion.off : Labels.motion.on;
            case "ultrasonic":
                return value ? Labels.ultrasonic.far : Labels.ultrasonic.close;
            case "alarm":
                return value ? Labels.alarm.off : Labels.alarm.on;
            case "door":
                return value ? Labels.door.opening : Labels.door.closing;
            case "ldr":
                return value ? Labels.ldr.off : Labels.ldr.on;
            default:
                return "unknown";
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
                <img alt="device icon" className="Image" src={imgSrc}></img>
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