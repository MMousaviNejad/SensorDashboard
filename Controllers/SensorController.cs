using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.SignalR;
using SensorDashboard.Hubs;
using SensorDashboard.Models;
using System.Diagnostics;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace SensorDashboard.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class SensorController : Controller
    {
        private readonly IHubContext<SensorHub> _hubContext;
        private static SensorData staticData = new SensorData();
        public SensorController(IHubContext<SensorHub> hubContext)
        {
            _hubContext = hubContext;
        }
        [HttpPost("update")]
        public async Task<IActionResult> UpdateSensorData([FromBody] SensorData data)
        {
            staticData = data;
            await _hubContext.Clients.All.SendAsync("ReceiveSensorData", data);
            return Ok(staticData);
        }

        [HttpPost("control")]
        public async Task<IActionResult> ControlRelay([FromBody] RelayCommand command)
        {
            staticData.Relay = command.Relay;
            await _hubContext.Clients.All.SendAsync("ReceiveSensorData", staticData);
            return Ok(new { Message = "Relay command received" });
        }

        [HttpGet("status")]
        public IActionResult GetSensorStatus()
        {
            return Ok(staticData);
        }
    }
}
