from fastapi import FastAPI, Request

app = FastAPI()

@app.post("/data")
async def receive_data(request: Request):
    data = await request.json()
    print(f"Humidity: {data['humidity']}%")
    print(f"Temperature: {data['temperature']}°{data['unit']}")
