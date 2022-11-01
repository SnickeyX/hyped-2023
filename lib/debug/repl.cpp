#include "repl.hpp"

#include <sstream>

namespace hyped::debug {
Repl::Repl(hyped::core::ILogger &log) : log_(log)
{
  addQuitCommand();
  addHelpCommand();
}

void Repl::addCommand(const Command &cmd)
{
  commands_.push_back(cmd);
  command_map_.emplace(cmd.name, cmd);
  log_.log(hyped::core::LogLevel::kDebug, "Added command: %s", cmd.name.c_str());
}

void Repl::addQuitCommand()
{
  addCommand(Command{"quit", "Quit the REPL", [this]() { exit(0); }});
}

void Repl::addHelpCommand()
{
  addCommand(Command{"help", "Print this help message", [this]() { printCommands(); }});
}

void Repl::addAdcCommands(const uint8_t pin)
{
    const auto adc = std::make_shared<hyped::io::Adc>(pin, log_);
    {
      Command command;
      std::stringstream identifier;
      identifier << "adc" << static_cast<int>(pin) << " read";
      command.name = identifier.str();
      std::stringstream description;
      description << "Read from ADC pin " << static_cast<int>(pin);
      command.description = description.str();
      command.callback = [this, adc]() {
        const auto value = adc->readValue();
        if (value) {
          log_.log(hyped::core::LogLevel::kInfo, "ADC value: %d", *value);
        } 
      };
    }
}

void Repl::printCommands()
{
  log_.log(hyped::core::LogLevel::kInfo, "Available commands:");
  for (const auto &cmd : commands_) {
    log_.log(hyped::core::LogLevel::kInfo, "  %s: %s", cmd.name.c_str(), cmd.description.c_str());
  }
}

void Repl::handleCommand()
{
  std::cout << "> ";
  std::string command;
  std::getline(std::cin, command);
  auto identifier = command_map_.find(command);
  if (identifier == command_map_.end()) {
    log_.log(hyped::core::LogLevel::kFatal, "Unknown command: %s", command.c_str());
    return;
  }
  identifier->second.callback();
}

void Repl::run()
{
  while (true) {
    handleCommand();
  }
}
}  // namespace hyped::debug