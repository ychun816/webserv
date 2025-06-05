#!/usr/bin/ruby

# Headers CGI obligatoires
puts "Content-Type: text/html\r"
puts "\r"

# Contenu HTML
puts "<html>"
puts "<head>"
puts "  <title>Test Ruby CGI</title>"
puts "  <style>"
puts "    body { font-family: Arial, sans-serif; margin: 20px; }"
puts "    h1 { color: #dc143c; }"
puts "    .info { background: #f0f0f0; padding: 10px; margin: 10px 0; }"
puts "  </style>"
puts "</head>"
puts "<body>"
puts "  <h1>🚀 Ruby CGI fonctionne !</h1>"
puts "  <div class='info'>"
puts "    <h3>Informations Ruby :</h3>"
puts "    <p><strong>Version Ruby :</strong> #{RUBY_VERSION}</p>"
puts "    <p><strong>Plateforme :</strong> #{RUBY_PLATFORM}</p>"
puts "    <p><strong>Date/Heure :</strong> #{Time.now}</p>"
puts "  </div>"

puts "  <div class='info'>"
puts "    <h3>Variables d'environnement CGI :</h3>"
puts "    <ul>"

# Afficher les variables d'environnement importantes
cgi_vars = [
  'REQUEST_METHOD',
  'REQUEST_URI',
  'QUERY_STRING',
  'SERVER_NAME',
  'SERVER_PORT',
  'SCRIPT_NAME',
  'SCRIPT_FILENAME',
  'REDIRECT_STATUS',
  'HTTP_USER_AGENT',
  'CONTENT_TYPE',
  'CONTENT_LENGTH'
]

cgi_vars.each do |var|
  value = ENV[var] || 'non défini'
  puts "      <li><strong>#{var} :</strong> #{value}</li>"
end

puts "    </ul>"
puts "  </div>"

# Si il y a une query string, l'afficher
if ENV['QUERY_STRING'] && !ENV['QUERY_STRING'].empty?
  puts "  <div class='info'>"
  puts "    <h3>Paramètres de requête :</h3>"
  puts "    <p>#{ENV['QUERY_STRING']}</p>"
  puts "  </div>"
end

puts "  <div class='info'>"
puts "    <h3>Test de fonctionnalités Ruby :</h3>"
puts "    <p><strong>Calcul :</strong> 2 + 2 = #{2 + 2}</p>"
puts "    <p><strong>Tableau :</strong> #{[1, 2, 3, 4, 5].join(', ')}</p>"
puts "    <p><strong>Hash :</strong> #{{'nom' => 'Ruby', 'type' => 'CGI'}.inspect}</p>"
puts "  </div>"

puts "  <p><em>Script généré par Ruby #{RUBY_VERSION} à #{Time.now}</em></p>"
puts "</body>"
puts "</html>"