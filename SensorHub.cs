using Microsoft.AspNetCore.SignalR;
using SensorDashboard.Models;

namespace SensorDashboard.Hubs
{
    public class SensorHub : Hub
    {
        // متد برای ارسال داده‌ها به تمام کلاینت‌های متصل
        public async Task SendSensorData(SensorData data)
        {
            await Clients.All.SendAsync("ReceiveSensorData", data);
        }
    }
}
