# python -m uvicorn main:app --reload
# pip install fastapi uvicorn pymongo
from fastapi import FastAPI
from fastapi import Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse
from fastapi.staticfiles import StaticFiles
from pymongo import DESCENDING
from datetime import datetime
from database import collection
from math import sqrt

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/data")
async def receive_data(request: Request):
    data = await request.json()
    
    print(f"Humidity: {data['humidity']}%")
    print(f"Temperature: {data['temperature']}°{data['unit']}")
    print(f"eCO2: {data['eCO2']} ppm")
    print(f"TVOC: {data['TVOC']} ppb")
    
    temperature = data.get("temperature")
    humidity = data.get("humidity")
    eco2 = data.get("eCO2") 
    tvoc = data.get("TVOC")  

    record = {
        "temperature": temperature,
        "humidity": humidity,
        "eco2": eco2,
        "tvoc": tvoc,
        "timestamp": datetime.utcnow()
    }

    await collection.insert_one(record)
    return {"message": "Data received and stored successfully"}

@app.get("/", response_class=HTMLResponse)
async def serve_chart():
    with open("index.html", "r") as f:
        return HTMLResponse(content=f.read())
    
@app.get("/latest-data")
async def get_latest_data():
    document = await collection.find_one(sort=[("timestamp", DESCENDING)])
    if not document:
        return {"temperature": None, "humidity": None, "eco2": None, "tvoc": None}

    return {
        "temperature": document["temperature"],
        "humidity": document["humidity"],
        "eco2": document["eco2"],
        "tvoc": document["tvoc"],
        "timestamp": document["timestamp"]
    }

@app.get("/temperature-stddev")
async def temperature_stddev():
    cursor = collection.find()
    data = [doc async for doc in cursor]
    if not data:
        return {"temperature_stddev": None}

    temperatures = [item["temperature"] for item in data]
    mean = sum(temperatures) / len(temperatures)
    variance = sum((t - mean) ** 2 for t in temperatures) / len(temperatures)
    stddev = round(sqrt(variance), 2)

    return {"temperature_stddev": stddev}

@app.get("/average-temperature")
async def average_temperature():
    cursor = collection.find()
    data = [doc async for doc in cursor]
    if not data:
        return {"average_temperature": None}
    avg = sum(item["temperature"] for item in data) / len(data)

    return {"average_temperature": round(avg, 2)}
