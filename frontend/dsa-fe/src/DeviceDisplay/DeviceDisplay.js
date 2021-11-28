import React from 'react'
import  Device from './Device/Device'
import './DeviceDisplay.css'

const DeviceDisplay = ({devices, sensors,handleConnect}) => {

    return(
        <div className="Container">
            {
            devices.map(dev =>
            <Device
                id={dev.id}
                name={dev.name}
                value={dev.value}
                key={dev.id}
                details={dev.details}
                sensor={sensors}
                handleConnect={handleConnect}
            />)
            }
        </div>
    );

}

export default DeviceDisplay;