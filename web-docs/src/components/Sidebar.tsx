import React, { useState } from 'react';
import { NAV_STRUCTURE } from '../data/docsData';
import { ChevronDown, ChevronRight, Compass, Terminal, Image, Sparkles, Wrench, Layers, Package, Users } from 'lucide-react';

interface SidebarProps {
  activeId: string;
  onSelect: (id: string) => void;
  isOpen: boolean;
}

const iconMap: Record<string, React.ReactNode> = {
  Compass: <Compass size={16} />,
  Terminal: <Terminal size={16} />,
  Image: <Image size={16} />,
  Sparkles: <Sparkles size={16} />,
  Wrench: <Wrench size={16} />,
  Layers: <Layers size={16} />,
  Package: <Package size={16} />,
  Users: <Users size={16} />,
};

export const Sidebar: React.FC<SidebarProps> = ({ activeId, onSelect, isOpen }) => {
  // All categories open by default
  const [openCategories, setOpenCategories] = useState<Record<string, boolean>>({
    'Getting Started': true,
    'Terminal & Shell': true,
    'Graphics & Artwork': true,
    'AI & Intelligence': true,
    'Developer Tooling': true,
    'Architecture & Core': true,
    'Packaging & Releases': true,
    'Community & Status': true,
  });

  const toggleCategory = (title: string) => {
    setOpenCategories((prev) => ({
      ...prev,
      [title]: !prev[title],
    }));
  };

  return (
    <aside className={`fedora-sidebar ${isOpen ? 'open' : ''}`}>
      <div className="sidebar-header">
        <span className="sidebar-product-label">Meridian Documentation</span>
        <span className="sidebar-version-badge">v2.0 (f44)</span>
      </div>

      <nav className="sidebar-nav">
        {NAV_STRUCTURE.map((category) => {
          const isCategoryOpen = openCategories[category.title] ?? true;
          const hasActiveItem = category.items.some((item) => item.id === activeId);

          return (
            <div key={category.title} className="nav-group">
              <button
                className={`nav-group-header ${hasActiveItem ? 'has-active' : ''}`}
                onClick={() => toggleCategory(category.title)}
              >
                <span className="nav-group-icon">
                  {iconMap[category.icon] || <Compass size={16} />}
                </span>
                <span className="nav-group-title">{category.title}</span>
                <span className="nav-group-chevron">
                  {isCategoryOpen ? <ChevronDown size={14} /> : <ChevronRight size={14} />}
                </span>
              </button>

              {isCategoryOpen && (
                <ul className="nav-sub-list">
                  {category.items.map((item) => {
                    const isActive = item.id === activeId;
                    return (
                      <li key={item.id}>
                        <button
                          className={`nav-sub-item ${isActive ? 'active' : ''}`}
                          onClick={() => onSelect(item.id)}
                        >
                          <span className="nav-item-bullet">•</span>
                          <span className="nav-item-text">{item.title}</span>
                          {item.status === 'experimental' && (
                            <span className="nav-status-tag tag-exp">exp</span>
                          )}
                          {item.status === 'development' && (
                            <span className="nav-status-tag tag-dev">dev</span>
                          )}
                        </button>
                      </li>
                    );
                  })}
                </ul>
              )}
            </div>
          );
        })}
      </nav>

      <div className="sidebar-footer">
        <a
          href="https://github.com/charanbalaji2005/Meridian-Shell"
          target="_blank"
          rel="noreferrer"
          className="sidebar-github-link"
        >
          <span>charanbalaji2005/Meridian-Shell</span>
        </a>
      </div>
    </aside>
  );
};
