using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.SignalR;
using SensorDashboard.Hubs;
using SensorDashboard.Models;

namespace SensorDashboard.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class SensorController : Controller
    {
        private readonly IHubContext<SensorHub> _hubContext;

        public SensorController(IHubContext<SensorHub> hubContext)
        {
            _hubContext = hubContext;
        }
        [HttpPost("update")]
        public async Task<IActionResult> UpdateSensorData([FromBody] SensorData data)
        {
            await _hubContext.Clients.All.SendAsync("ReceiveSensorData", data);
            return Ok("Data received");
        }
    }
}
